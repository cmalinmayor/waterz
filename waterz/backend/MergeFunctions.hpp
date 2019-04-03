#ifndef MERGE_FUNCTIONS_H__
#define MERGE_FUNCTIONS_H__

#include "RegionSizeProvider.hpp"
#include "ContactAreaProvider.hpp"
#include "MinAffinityProvider.hpp"
#include "MaxAffinityProvider.hpp"
#include "MeanAffinityProvider.hpp"
#include "HistogramQuantileProvider.hpp"
#include "VectorQuantileProvider.hpp"
#include "MaxKAffinityProvider.hpp"
#include "RandomNumberProvider.hpp"
#include "ConstantProvider.hpp"
#include "MergeProviders.hpp"

/**
 * Scores edges with a constant.
 */
template <typename RegionGraphType, int C>
class Constant {

public:

	typedef ConstantProvider<C> StatisticsProviderType;
	typedef typename RegionGraphType::EdgeIdType EdgeIdType;
	typedef typename StatisticsProviderType::ValueType ScoreType;

	Constant(
			RegionGraphType& regionGraph) :
		_constantProvider(ConstantProvider<C>(regionGraph)) {}

	inline ScoreType operator()(EdgeIdType e) const {

		return _constantProvider();
	}

private:

	const StatisticsProviderType& _constantProvider;
};

/**
 * Scores edges with min size of incident regions.
 */
template <typename RegionGraphType>
class MinSize {

public:

	typedef RegionSizeProvider<RegionGraphType> StatisticsProviderType;
	typedef typename RegionGraphType::EdgeIdType EdgeIdType;
	typedef typename StatisticsProviderType::ValueType ScoreType;

	MinSize(
			RegionGraphType& regionGraph,
			const StatisticsProviderType& regionSizeProvider) :
		_regionGraph(regionGraph),
		_regionSizeProvider(regionSizeProvider) {}

	inline ScoreType operator()(EdgeIdType e) {

		return std::min(
				_regionSizeProvider[_regionGraph.edge(e).u],
				_regionSizeProvider[_regionGraph.edge(e).v]);
	}

private:

	const RegionGraphType&    _regionGraph;
	const StatisticsProviderType& _regionSizeProvider;
};

/**
 * Scores edges with max size of incident regions.
 */
template <typename RegionGraphType>
class MaxSize {

public:

	typedef RegionSizeProvider<RegionGraphType> StatisticsProviderType;
	typedef typename RegionGraphType::EdgeIdType EdgeIdType;
	typedef typename StatisticsProviderType::ValueType ScoreType;

	MaxSize(
			RegionGraphType& regionGraph,
			const StatisticsProviderType& regionSizeProvider) :
		_regionGraph(regionGraph),
		_regionSizeProvider(regionSizeProvider) {}

	inline ScoreType operator()(EdgeIdType e) {

		return std::max(
				_regionSizeProvider[_regionGraph.edge(e).u],
				_regionSizeProvider[_regionGraph.edge(e).v]);
	}

private:

	const RegionGraphType& _regionGraph;
	const StatisticsProviderType& _regionSizeProvider;
};

/**
 * Directly use a statistic from a provider for edges.
 */
template <typename RegionGraphType, typename StatisticsProviderTypeType>
class EdgeStatisticValue {

public:

	typedef StatisticsProviderTypeType StatisticsProviderType;
	typedef typename RegionGraphType::EdgeIdType EdgeIdType;
	typedef typename StatisticsProviderType::ValueType ScoreType;

	EdgeStatisticValue(
			RegionGraphType&,
			const StatisticsProviderType& provider) :
		_provider(provider) {}

	inline ScoreType operator()(EdgeIdType e) {

		return _provider[e];
	}

private:

	const StatisticsProviderType& _provider;
};

template <typename RegionGraphType, typename Precision>
using MinAffinity = EdgeStatisticValue<RegionGraphType, MinAffinityProvider<RegionGraphType, Precision>>;

template <typename RegionGraphType, typename Precision>
using MaxAffinity = EdgeStatisticValue<RegionGraphType, MaxAffinityProvider<RegionGraphType, Precision>>;

template <typename RegionGraphType, typename Precision>
using MeanAffinity = EdgeStatisticValue<RegionGraphType, MeanAffinityProvider<RegionGraphType, Precision>>;

template <typename RegionGraphType, typename QuantileFunction, typename Precision, int Bins, bool InitWithMax = true>
class HistogramQuantileFunctionAffinity {

public:

	typedef HistogramQuantileProvider<RegionGraphType, QuantileFunction, Precision, Bins, InitWithMax> HistogramProviderType;
	typedef typename QuantileFunction::StatisticsProviderType QuantileFunctionProviderType;
	typedef typename MergeProviders<
				HistogramProviderType,
				QuantileFunctionProviderType>::Value
			StatisticsProviderType;

	typedef typename RegionGraphType::EdgeIdType EdgeIdType;
	typedef Precision ScoreType;

	HistogramQuantileFunctionAffinity(
			RegionGraphType&,
			const StatisticsProviderType& mergedStatistics) :
		_histogramProvider(mergedStatistics) {}

	inline ScoreType operator()(EdgeIdType e) {

		return _histogramProvider[e];
	}

private:

	const HistogramProviderType& _histogramProvider;
};

template <typename RegionGraphType, int Quantile, typename Precision, int Bins, bool InitWithMax = true>
using HistogramQuantileAffinity = HistogramQuantileFunctionAffinity<
	RegionGraphType,
	Constant<RegionGraphType, Quantile>,
	Precision,
	Bins,
	InitWithMax
>;

template <typename RegionGraphType, int Quantile, typename Precision, bool InitWithMax = true>
using QuantileAffinity = EdgeStatisticValue<RegionGraphType, VectorQuantileProvider<RegionGraphType, Quantile, Precision, InitWithMax>>;

/**
 * Scores edges with the mean of the max k affinities.
 */
template <typename RegionGraphType, int K, typename Precision>
class MeanMaxKAffinity {

public:

	typedef MaxKAffinityProvider<RegionGraphType, K, Precision> StatisticsProviderType;
	typedef typename RegionGraphType::EdgeIdType EdgeIdType;
	typedef Precision ScoreType;

	MeanMaxKAffinity(
			RegionGraphType&,
			const StatisticsProviderType& maxKAffinityProvider) :
		_maxKAffinityProvider(maxKAffinityProvider) {}

	inline ScoreType operator()(EdgeIdType e) {

		return _maxKAffinityProvider[e].average();
	}

private:

	const StatisticsProviderType& _maxKAffinityProvider;
};

/**
 * Scores edges with the size of the contact area.
 */
template <typename RegionGraphType>
using ContactArea = EdgeStatisticValue<RegionGraphType, ContactAreaProvider<RegionGraphType>>;

/**
 * Scores edges with a random number between 0 and 1.
 */
template <typename RegionGraphType>
class Random {

public:

	typedef RandomNumberProvider StatisticsProviderType;
	typedef typename RegionGraphType::EdgeIdType EdgeIdType;
	typedef typename StatisticsProviderType::ValueType ScoreType;

	Random(
			RegionGraphType&,
			const StatisticsProviderType& randomProvider) :
		_randomProvider(randomProvider) {}

	inline ScoreType operator()(EdgeIdType e) {

		return _randomProvider();
	}

private:

	const StatisticsProviderType& _randomProvider;
};

#endif // MERGE_FUNCTIONS_H__


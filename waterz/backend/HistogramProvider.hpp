#ifndef WATERZ_HISTOGRAM_QUANTILE_PROVIDER_H__
#define WATERZ_HISTOGRAM_QUANTILE_PROVIDER_H__

#include "StatisticsProvider.hpp"
#include "Histogram.hpp"
#include "discretize.hpp"

/**
 * A histogram provider that assumes that all values are in the range [0,1].
 * Allows upstream manipulation based on other edge scores.
 */
template <typename RegionGraphType, typename Precision, int Bins = 256, bool InitWithMax = true>
class HistogramProvider : public StatisticsProvider {

public:

	typedef Histogram<Bins> ValueType;
	typedef typename RegionGraphType::EdgeIdType EdgeIdType;

	HistogramQuantileProvider(RegionGraphType& regionGraph) :
		_histograms(regionGraph) {}

	inline void addAffinity(EdgeIdType e, ValueType affinity) {

		int bin = discretize<int>(affinity, Bins);

		if (InitWithMax && _histograms[e].lowestBin() != Bins) {

			if (bin > _histograms[e].lowestBin())
				_histograms[e].clear();
			else
				return;
		}

		_histograms[e].inc(bin);
	}

	inline bool notifyEdgeMerge(EdgeIdType from, EdgeIdType to) {

		_histograms[to] += _histograms[from];
		_histograms[from].clear();

		return true;
	}

	inline ValueType operator[](EdgeIdType e) const {
		return _histograms[e];
	}

private:

	typename RegionGraphType::template EdgeMap<Histogram<Bins>> _histograms;
};

#endif // WATERZ_HISTOGRAM_QUANTILE_PROVIDER_H__


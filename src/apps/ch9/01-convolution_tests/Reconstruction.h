#pragma once


#include <functional>
#include <vector> // also provides std::size

#include <cassert>
#include <cmath>
#include <cstddef>


namespace ad {
namespace focg {


template <class T_value>
struct Filter
{
    // Proxy for calling filter
    T_value operator()(double x) const
    {
        return filter(x);
    }

    std::function<T_value(double)> filter;
    double radius;
};


template <class T_sequence, class T_value = typename T_sequence::value_type>
T_value reconstruct1D(T_sequence aSequence, Filter<T_value> aFilter, double x)
{
    assert(x >= 0.
           && x <= (std::size(aSequence) - 1));

    const double r = aFilter.radius;

    T_value accum{}; // zero init
    // IMPORTANT FoCG 3rd p200:
    // notes that if a point fall exactly at distance r from x, it will be left out
    // Not sure if that is valid only for lower bound ? both bounds ?
    for (std::size_t i = std::max<double>(std::ceil(x - r), 0);
         i <= std::min<std::size_t>(std::floor(x + r), std::size(aSequence) - 1);
         ++i)
    {
        accum += aSequence[i] * aFilter(x - i);
    }
    return accum;
}


// Non-sense
//template <class T_sequence, class T_value = typename T_sequence::value_type>
//std::vector<T_value> reconstructSequence1D(T_sequence aSequence, Filter<T_value> aFilter)
//{
//    std::vector<T_value> result;
//    for (std::size_t i = 0; i != std::size(aSequence); ++i)
//    {
//        result.push_back(reconstructDiscrete1D(aSequence, aFilter, i));
//    }
//    return result;
//}

}// namespace focg
} // namespace ad

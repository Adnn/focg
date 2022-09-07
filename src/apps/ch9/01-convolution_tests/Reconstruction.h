#pragma once


#include "Convolution.h"

#include <functional>
#include <vector> // also provides std::size

#include <cassert>
#include <cmath>
#include <cstddef>


namespace ad {
namespace focg {


struct Filter
{
    // Proxy for calling filter
    double operator()(double x) const
    {
        return filter(x);
    }

    std::function<double(double)> filter;
    double radius;
};


template <class T_sequence, class T_value = typename T_sequence::value_type>
T_value reconstruct1D(T_sequence aSequence, Filter aFilter, double x)
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


template <int N, class T>
math::Size<2, std::size_t> dimensions(const math::Matrix<N, N, T> &)
{
    return math::Size<2, std::size_t>{(std::size_t)N, (std::size_t)N};
}


template <class T_pixelFormat>
math::Size<2, std::size_t> dimensions(const arte::Image<T_pixelFormat> & a)
{
    return math::Size<2, std::size_t>{a.dimensions()};
}


template <class T_inSequence2D, class T_outSequence2D, class T_value = typename T_inSequence2D::value_type>
void resampleSeparable2D(const T_inSequence2D & aInput,
                         T_outSequence2D & aOutput,
                         Filter aFilter)
{
    const double r = aFilter.radius;

    math::Vec<2, double> delta = 
        math::Vec<2, double>{dimensions(aInput)}.cwDiv(math::Vec<2, double>{dimensions(aOutput)});

    // TODO this is quite coupled to the fact that the sequences are images
    auto intermediary = arte::Image<T_value>::makeUninitialized({(int)width(aOutput), (int)height(aInput)});

    // With the convention that the image domain is (-0.5, Nx - 0.5) x (-0.5, Ny - 0.5)
    double x0 = -0.5 + delta.x()/2;
    double y0 = -0.5 + delta.y()/2;

    // Resample all the rows of the source
    for (std::size_t i = 0; i != height(aInput); ++i)
    {
        // For each pixel in the (intermediary) output row
        for (std::size_t j = 0; j != width(aOutput); ++j)
        {
            intermediary[j][i] = T_value{}; // assign zero
            // x coordinate of the output pixel, expressed in the input grid
            double x = x0 + j * delta.x();
            // For each pixel **center** which falls within the radius of the filter
            // (the filter being centered on x, i.e. the output pixel)
            for (std::size_t k = std::max<double>(std::ceil(x - r), 0);
                 k <= std::min<std::size_t>(std::floor(x + r), width(aInput) - 1);
                 ++k)
            {
                intermediary[j][i] += aInput[k][i] * aFilter(x - k);
            }
        }
    }

    // Resample all the columns of the intermediary
    for (std::size_t j = 0; j != width(aOutput); ++j)
    {
        for (std::size_t i = 0; i != height(aOutput); ++i)
        {
            aOutput[j][i] = T_value{}; // assign zero
            double y = y0 + i * delta.y();
            for (std::size_t k = std::max<double>(std::ceil(y - r), 0);
                 k <= std::min<std::size_t>(std::floor(y + r), height(intermediary) - 1);
                 ++k)
            {
                aOutput[j][i] += intermediary[j][k] * aFilter(y - k);
            }
        }
    }
}


template <class T_inSequence2D, class T_outSequence2D, class T_value = typename T_inSequence2D::value_type>
void pointResample2D(const T_inSequence2D & aInput, T_outSequence2D & aOutput)
{
    math::Vec<2, double> delta = 
        math::Vec<2, double>{dimensions(aInput)}.cwDiv(math::Vec<2, double>{dimensions(aOutput)});

    // With the convention that the image domain is (-0.5, Nx - 0.5) x (-0.5, Ny - 0.5)
    double x0 = -0.5 + delta.x() / 2;
    double y0 = -0.5 + delta.y() / 2;

    // Resample all the rows of the source
    for (std::size_t i = 0; i != height(aOutput); ++i)
    {
        double y = y0 + i * delta.y();
        for (std::size_t j = 0; j != width(aOutput); ++j)
        {
            double x = x0 + j * delta.x();
            aOutput[j][i] = aInput[std::round(x)][std::round(y)];
        }
    }
}

}// namespace focg
} // namespace ad

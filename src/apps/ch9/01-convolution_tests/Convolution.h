#pragma once


#include <arte/Image.h>

#include <math/Matrix.h>
#include <math/Rectangle.h>
#include <math/Vector.h>

#include <vector> // also provides std::size

#include <cassert>
#include <cstddef>


namespace ad {
namespace focg {


template <class T_leftSequence, class T_rightSequence, class T_value = typename T_rightSequence::value_type>
T_value convolveDiscrete1D(T_leftSequence a, T_rightSequence b, std::size_t i)
{
    // i must be within b (ensuring a mid element can never be clamped out).
    assert(i < std::size(b));

    // we require that sequence a has a middle element
    assert(std::size(a) % 2 == 1);
    // The radius is also the index of the middle element of a
    std::size_t r = std::size(a) / 2;

    // Implement clamping at b edges
    // Number of elements between b first-element and i
    std::size_t bLeft = i;
    // Number of elements between i and b last-element
    std::size_t bRight = (/*last*/std::size(b) - 1) - i;

    T_value accu{}; // zero init
    // Attention: bRight (resp. bLeft) is used for left (resp. right) limit of the filter.
    // This is because convolution "crosses" the filter over the sequence.
    for(std::size_t j = r - std::min(bRight, r); // equivalent to std::max(r - bRight, 0), but avoid substraction underflow
        j <= r + std::min(bLeft, r);
        ++j)
    {
        // remember that when j == r, this is the "middle" of sequence a
        // i.e. the value to multiply with b[i]
        // so we add `r` to `b` indexing.
        accu += a[j] * b[i - j + r];
    }
    return accu;
}


template <class T_leftSequence, class T_rightSequence, class T_value = typename T_rightSequence::value_type>
std::vector<T_value> convolveSequence1D(T_leftSequence a, T_rightSequence b)
{
    std::vector<T_value> result;
    for (std::size_t i = 0; i != std::size(b); ++i)
    {
        result.push_back(convolveDiscrete1D(a, b, i));
    }
    return result;
}


template <class T_value>
constexpr std::array<T_value, 1> gDiscreteImpule{T_value{1}};


//
// 2D
//
template <std::size_t N_rows, std::size_t N_cols, class T_value>
math::Rectangle<std::size_t> rectangle(const math::Matrix<N_rows, N_cols, T_value> & a)
{
    return {
        {0u, 0u},
        // Invert here, to respect the convention width then height
        {N_cols, N_rows},
    };
}

template <std::size_t N_rows, std::size_t N_cols, class T_value>
std::size_t width(const math::Matrix<N_rows, N_cols, T_value> & a)
{
    return N_cols;
}

template <std::size_t N_rows, std::size_t N_cols, class T_value>
std::size_t height(const math::Matrix<N_rows, N_cols, T_value> & a)
{
    return N_rows;
}


template <class T_pixelFormat>
std::size_t width(const arte::Image<T_pixelFormat> & a)
{
    return a.width();
}


template <class T_pixelFormat>
std::size_t height(const arte::Image<T_pixelFormat> & a)
{
    return a.height();
}


template <std::size_t N_leftSize, class T_value, class T_rightSequence2D>
T_value convolveDiscrete2D(const math::Matrix<N_leftSize, N_leftSize, T_value> & a,
                           const T_rightSequence2D & b,
                           math::Position<2, std::size_t> aPosition)
{
    // aPosition must be within b (ensuring a mid element can never be clamped out).
    assert(rectangle(b).contains(aPosition));

    // we require that sequence a has a middle element
    static_assert(N_leftSize % 2 == 1, "Filter must have a middle element.");
    // The radius is also the index of the middle element of a
    constexpr std::size_t r = N_leftSize / 2;

    // Implement clamping at b edges
    // Number of elements between b left-element and i
    std::size_t bLeft = aPosition.x();
    // Number of elements between i and b right-element
    std::size_t bRight = (/*last*/width(b) - 1) - aPosition.x();

    std::size_t bBottom = aPosition.y();
    std::size_t bTop = (height(b) - 1) - aPosition.y();

    T_value accu{}; // zero init
    for(std::size_t i = r - std::min(bRight, r);
        i <= r + std::min(bLeft, r);
        ++i)
    {
        for(std::size_t j = r - std::min(bTop, r);
            j <= r + std::min(bBottom, r);
            ++j)
        {
            // remember that when i == j == r, this is the "middle" of sequence a
            // i.e. the value to multiply with b[i]
            // so we add `r` to `b` indexing.
            accu += a[i][j] * b[aPosition.x() - i + r][aPosition.y() - j + r];
        }
    }
    return accu;
}


template <std::size_t N_leftSize, class T_value, class T_rightSequence2D>
T_rightSequence2D convolveSequence2D(const math::Matrix<N_leftSize, N_leftSize, T_value> & a,
                                     const T_rightSequence2D & b)
{
    // Copy b, so it is of correct dimension
    T_rightSequence2D result{b};

    for (std::size_t i = 0; i != width(b); ++i)
    {
        for (std::size_t j = 0; j != height(b); ++j)
        {
            result[i][j] = convolveDiscrete2D(a, b, {i, j});
        }
    }
    return result;
}


/// \note Implement "border" sampling for edges.
template <class T_leftSequence1D, class T_rightSequence2D,
          class T_value = typename T_rightSequence2D::value_type>
T_rightSequence2D filterSeparable2D(const T_leftSequence1D & aFilter,
                                    const T_rightSequence2D & aSequence)
{
    // we require that sequence a has a middle element
    assert(std::size(aFilter) % 2 == 1);
    // The radius is also the index of the middle element of a
    std::size_t r = std::size(aFilter) / 2;

    // Sequence must be at least as large as filter
    assert(width(aSequence) >= std::size(aFilter) 
           && height(aSequence) >= std::size(aFilter));

    // Copy sequence, so it is of correct dimension
    T_rightSequence2D result{aSequence};
    std::vector S(width(aSequence), T_value{});

    // For each line
    for(std::size_t y = 0; y != height(aSequence); ++y)
    {
        // Cache S for the line
        for(std::size_t x = 0; x != width(aSequence); ++x)
        {
            S[x] = T_value{}; // set to zero
            for(std::size_t j = 0; j != std::size(aFilter); ++j)
            {
                // Implement "border" sampling for edges (i.e., sample the border value for "out of domain")
                // clamping (y - j + r) to [0, height-1]
                // note that max(j) is 2r
                std::size_t yIndex = std::clamp(y, r, height(aSequence) - 1 - r) - j + r;
                S[x] += aFilter[j] * aSequence[x][yIndex];
            }
        }

        // For each column
        for(std::size_t x = 0; x != width(aSequence); ++x)
        {
            result[x][y] = T_value{}; // set to zero
            // Compute filtered value
            for(std::size_t i = 0; i != std::size(aFilter); ++i)
            {
                // Implement "border" sampling for edges
                // clamping (x - i + r) to [0, width-1]
                std::size_t xIndex = std::clamp(x, r, width(aSequence) - 1 - r) - i + r;
                result[x][y] += aFilter[i] * S[xIndex];
            }
        }
    }

    return result;
}


}// namespace focg
} // namespace ad

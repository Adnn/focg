#pragma once


#include <array>
#include <numbers>

#include <cmath>


namespace ad {
namespace focg {


template <class T_value = double>
T_value gaussian(T_value x, T_value sigma = 1)
{
    return T_value{1} / (sigma * std::sqrt(2 * std::numbers::pi_v<T_value>))
        * std::exp(-std::pow(x, 2) / (2 * std::pow(sigma, 2)));
}


template <unsigned int N_radius, class T_value = double>
std::array<T_value, 2 * N_radius + 1> discreteGaussian(T_value sigma = 1, T_value scale = 1)
{
    std::array<T_value, 2 * N_radius + 1> result;
    int v = -(int)N_radius;
    int v2 = -static_cast<int>(N_radius);
    for (int i = -(int)N_radius; i <= (int)N_radius; ++i)
    {
        result[i + (int)N_radius] = gaussian(static_cast<T_value>(i) / scale, sigma) / scale;
    }
    return result;
}


// See FoCG 3rd p209
// I am not 100% sur of my understanding of the discrete impulse in this context, but it does sharpen the image.
template <unsigned int N_radius, class T_value = double>
std::array<T_value, 2 * N_radius + 1> discreteSharpen(T_value alpha, T_value sigma = 1, T_value scale = 1)
{
    auto result = discreteGaussian<N_radius>(sigma, scale);
    for (auto & v : result)
    {
        v *= -alpha;
    }
    result[N_radius] += 1 + alpha;
    return result;
}


} // namespace focg
} // namespace ad

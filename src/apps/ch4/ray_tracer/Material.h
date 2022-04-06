#pragma once

#include <math/Color.h>


namespace ad {
namespace focg {


struct Material
{
    math::hdr::Rgb_d ambientColor;
    math::hdr::Rgb_d diffuseColor;
    math::hdr::Rgb_d specularColor;
    int phongExponent{1}; // control specular fall-off
    math::hdr::Rgb_d reflectionColor{math::hdr::gBlack<>};
};


} // namespace focg
} // namespace ad

#pragma once

#include <math/Color.h>


namespace ad {
namespace focg {


struct Material
{
    math::hdr::Rgb ambientColor;
    math::hdr::Rgb diffuseColor;
    math::hdr::Rgb specularColor;
    int phongExponent{1}; // control specular fall-off
    math::hdr::Rgb reflectionColor{math::hdr::gBlack};
};


} // namespace focg
} // namespace ad

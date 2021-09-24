#pragma once


#include "Scene.h"

#include <arte/Image.h>

#include <math/Homogeneous.h>


namespace ad {
namespace focg {


ad::Image<> rasterize(const Scene & aScene, math::Matrix<4, 4> aTransformation, math::Size<2, int> aResolution)
{
    ad::Image<> image{aResolution, math::sdr::gBlack};

    for (const auto & line : aScene.lines)
    {
        drawLine(line * aTransformation, image);
    }

    return image;
}


} // namespace focg
} // namespace ad

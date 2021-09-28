#pragma once


#include "Clipping.h"
#include "Rasterization.h"
#include "Scene.h"


namespace ad {
namespace focg {


inline Image<> traversePipeline(const Scene & aScene, math::Size<2, int> aResolution)
{
    ad::Image<> image{aResolution, math::sdr::gBlack};
    ViewVolume volume{math::Box<double>{
        // Important 0.5 offset, because the integer coordinate a pixel centers !
        // there is nonetheless an issue, since -0.5 rounds to -1 and 0.5 rounds to 1
        // (which tends to include adjacent pixels when clipping, thus overlapping to next line/column)
        {-0.5, -0.5, 500.},
        {static_cast<ad::math::Size<2, double>>(aResolution), 500.},
    }};

    for (const auto & line : aScene.lines)
    {
        if (auto clippedLine = clip(line, volume))
        {
            rasterizeLine(*clippedLine, image, ad::math::sdr::gWhite);
            //rasterizeLine(line, image, ad::math::sdr::gWhite);
        }
    }

    return image;
}

 
} // namespace focg
} // namespace ad

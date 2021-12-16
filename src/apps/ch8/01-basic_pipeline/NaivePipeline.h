#pragma once


#include "Clipping.h"
#include "Rasterization.h"
#include "Scene.h"

#include <arte/Image.h>

#include <math/Vector.h>

#include <bitset>


namespace ad {
namespace focg {


struct NaivePipeline
{
private:
    using RenderFlag = std::bitset<2>;

public:
    arte::Image<math::sdr::Rgb> traverse(const Scene & aScene, math::Size<2, int> aResolution) const;

    static constexpr RenderFlag Wireframe = 0b01;
    static constexpr RenderFlag Fill = 0b10;
    RenderFlag renderMode{Fill};
    bool depthTesting;
    arte::Image<double> depthBuffer;
};


inline arte::Image<math::sdr::Rgb> NaivePipeline::traverse(const Scene & aScene, math::Size<2, int> aResolution) const
{
    arte::Image<math::sdr::Rgb> image{aResolution, math::sdr::gBlack};

    ViewVolume volume{math::Box<double>{
        // Important 0.5 offset, because the integer coordinate are at pixel centers!
        // there is nonetheless an issue, since -0.5 rounds to -1 and 0.5 rounds to 1
        // (which tends to include adjacent pixels when clipping, thus overlapping to next line/column)
        // changing from std::round to std::nearbyint seems to handle -0.5 and 0.5 as we need, but
        // rounding errors make it not robust enough at the moment.
        //{-0.5, -0.5, 500.},
        //{static_cast<ad::math::Size<2, double>>(aResolution), 500.},

        // A Q&D solution is to restrain the view volume by some arbitrary amount
        {-0.45, -0.45, 500.},
        {static_cast<math::Size<2, double>>(aResolution) - math::Size<2, double>{0.1, 0.1}, 1000.},
    }};

    for (const auto & line : aScene.lines)
    {
        if (auto clippedLine = clip(line, volume))
        {
            rasterizeLine(*clippedLine, image, ad::math::sdr::gWhite);
        }
    }

    for (const auto & triangle : aScene.triangles)
    {
        for (const auto & triangle : clip(triangle, volume))
        {
            if ((renderMode & Fill).any())
            {
                rasterizeIncremental(triangle, image);
                //rasterize(triangle, image);
                //rasterizeBis(triangle, image);
            }
            if ((renderMode & Wireframe).any())
            {
                rasterizeLine(triangle.getLineC(), image);
                rasterizeLine(triangle.getLineB(), image);
                rasterizeLine(triangle.getLineA(), image);
            }
        }
    }

    return image;
}

 
} // namespace focg
} // namespace ad

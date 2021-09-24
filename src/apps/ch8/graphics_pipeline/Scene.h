#pragma once


#include "Line.h"
#include "MidpointAlgorithm.h"

#include <arte/Image.h>

#include <vector>


namespace ad {
namespace focg {


struct Scene
{
    Image<> render(math::Size<2, int> aResolution);

    std::vector<Line> lines;
};

inline Image<> Scene::render(math::Size<2, int> aResolution)
{
    ad::Image<> image{aResolution, math::sdr::gBlack};

    for (const auto & line : lines)
    {
        rasterizeLine(line, image);
    }

    return image;
}

} // namespace focg
} // namespace ad

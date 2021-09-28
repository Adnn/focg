#pragma once


#include "Clipping.h"
#include "Line.h"
#include "Triangle.h"
#include "Rasterization.h"

#include <arte/Image.h>

#include <vector>


namespace ad {
namespace focg {


struct Scene
{
    Image<> render(math::Size<2, int> aResolution);

    std::vector<Line> lines;
    std::vector<Triangle> triangles;
};

inline Image<> Scene::render(math::Size<2, int> aResolution)
{
    ad::Image<> image{aResolution, math::sdr::gBlack};

    for (const auto & line : lines)
    {
        rasterizeLine(line, image);
    }

    for (const auto & triangle : triangles)
    {
        rasterizeIncremental(triangle, image);
    }

    return image;
}

} // namespace focg
} // namespace ad

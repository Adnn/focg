#pragma once


#include "Clipping.h"
#include "Line.h"
#include "Triangle.h"
#include "Rasterization.h"

#include <arte/Image.h>

#include <math/Vector.h>

#include <vector>


namespace ad {
namespace focg {


template <class T_vertex>
struct Scene_base
{
    arte::Image<> render(math::Size<2, int> aResolution);

    std::vector<Line> lines;
    std::vector<Triangle_base<T_vertex>> triangles;
};


using Scene = Scene_base<Vertex>;


inline ad::arte::Image<> Scene::render(math::Size<2, int> aResolution)
{
    ad::arte::Image<> image{aResolution, math::sdr::gBlack};

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

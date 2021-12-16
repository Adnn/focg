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
struct Scene
{
    arte::Image<math::sdr::Rgb> render(math::Size<2, int> aResolution);

    std::vector<Line> lines;
    std::vector<Triangle<T_vertex>> triangles;
};


} // namespace focg
} // namespace ad

#pragma once


#include "Line.h"
#include "Triangle.h"

#include <arte/Image.h>

#include <math/Barycentric.h>
#include <math/Color.h>
#include <math/Vector.h>

#include <utility>


namespace ad {
namespace focg {


// Notes:
// Midpoint algorithm, FoCG 3rd 8.1.1 p165
//
// There is an important subtelty with the last point on the line:
// The endpoint coordinates are floating point values, so it can lie anywhere in the pixel.
// If it were to happen "early" (i.e. barely entering the last pixel),
// the midpoint test would virtually extend the line to the center of the pixel on the main axis,
// (main axis being the axis that is [in|de]cremented at each test, the secondary axis has a condition.)
// which could actually extend the line to a neighboring pixel on the secondary axis due to slope.
// Even though this respects the correct slope of the line, this has two issues:
// * it generates a fragment in a pixel that the line never actually enters.
// * if the endpoint is on the raster edge (e.g. after clipping), it could lead to an out-of-bounds access.
// For theses reasons, the endpoint is handled separately, after the main loop.
// This is actually symmetrical to what happens for the first pixel (which is shaded without considering slope).

// TODO color interpolation along the line
template <class T_raster>
void rasterizeLine(
    Line aLine,
    T_raster & aRaster,
    const math::sdr::Rgb aColor = math::sdr::gWhite)
{
    HPos & a = aLine.pointA;
    HPos & b = aLine.pointB;

    if (a.x() > b.x())
    {
        std::swap(a, b);
    }
    // Make sure to get the equation after potentially swapping points.
    auto f = aLine.getImplicitEquation();

    double m = (b.y() - a.y()) / (b.x() - a.x());
    // The four cases depending on the slope
    if (m <= -1)
    {
        int x = std::nearbyint(a.x());
        for (int y = std::nearbyint(a.y()); y > std::nearbyint(b.y()); --y)
        {
            aRaster.at(x, y) = aColor;
            if (f({x+0.5, y-1., 0., 1.}) < 0)
            {
                ++x;
            }
        }
    }
    else if (m <= 0)
    {
        int y = std::nearbyint(a.y());
        for (int x = std::nearbyint(a.x()); x < std::nearbyint(b.x()); ++x)
        {
            aRaster.at(x, y) = aColor;
            if (f({x+1., y-0.5, 0., 1.}) > 0)
            {
                --y;
            }
        }
    }
    else if (m <= 1)
    {
        int y = std::nearbyint(a.y());
        for (int x = std::nearbyint(a.x()); x < std::nearbyint(b.x()); ++x)
        {
            aRaster.at(x, y) = aColor;
            if (f({x+1., y+0.5, 0., 1.}) < 0)
            {
                ++y;
            }
        }
    }
    else
    {
        int x = std::nearbyint(a.x());
        for (int y = std::nearbyint(a.y()); y < std::nearbyint(b.y()); ++y)
        {
            aRaster.at(x, y) = aColor;
            if (f({x+0.5, y+1., 0., 1.}) > 0)
            {
                ++x;
            }
        }
    }
    // Separate handling of last point of the line (see notes above).
    aRaster.at(std::nearbyint(b.x()), std::nearbyint(b.y())) = aColor;
}


//
// Triangles
//

// Notes:
// Implementations following FoCG 3rd 8.1.2 p166.
//
// The rasterization only generate a fragment if the pixel center is inside the triangle 
// (or exactly on its edge).


/// \note aRaster is passed in because of legacy API of NaivePipeline,
/// otherwise it makes more sense that the fragment callback knows the target.
template <class T_vertex, class T_raster, class F_postRasterization>
void rasterizeIncremental(const Triangle<T_vertex> & aTriangle, 
                          T_raster & aRaster,
                          const F_postRasterization & aFragmentCallback)
{
    // Use to assign exactly tangent pixels on adjacent triangles (p 168)
    const HPos offscreenPoint{-1., -1., 0., 1.};

    // Setup
    auto fa = aTriangle.getFa();
    auto fb = aTriangle.getFb();
    auto fc = aTriangle.getFc();

    // It is important that the initial numerators are generated
    // for pixel center (so rounding has to happen now).
    const double xMinRound = std::nearbyint(aTriangle.xmin());
    const double yMinRound = std::nearbyint(aTriangle.ymin());

    math::Vec<3, double> previousNumerators{
        fa({xMinRound, yMinRound, 0., 1.}),
        fb({xMinRound, yMinRound, 0., 1.}),
        fc({xMinRound, yMinRound, 0., 1.}) 
    };

    const math::Vec<3, double> denominators{
        fa(aTriangle.a),
        fb(aTriangle.b),
        fc(aTriangle.c)
    };

    // Test for degenerate triangle (zero area), which should not be rasterized.
    if (denominators.x() == 0. || denominators.y() == 0. || denominators.z() == 0.)
    {
        return; 
    }

    const math::Vec<3> xIncrements{
        aTriangle.getLineA().getEquationFactorX(),
        aTriangle.getLineB().getEquationFactorX(),
        aTriangle.getLineC().getEquationFactorX()
    };

    const math::Vec<3> yIncrements{
        aTriangle.getLineA().getEquationFactorY(),
        aTriangle.getLineB().getEquationFactorY(),
        aTriangle.getLineC().getEquationFactorY()
    };

    math::Vec<3, double> numerators = previousNumerators;

    // Increments
    for (auto y = static_cast<int>(yMinRound);
         y <= static_cast<int>(std::nearbyint(aTriangle.ymax()));
         ++y)
    {
        for (auto x = static_cast<int>(xMinRound);
             x <= static_cast<int>(std::nearbyint(aTriangle.xmax()));
             ++x)
        {
            math::Vec<3, double> barycentric = numerators.cwDiv(denominators);

            // No structured binding on vectors at the moment
            double alpha = barycentric.x();
            double beta  = barycentric.y();
            double gamma = barycentric.z();

            if (alpha >= 0. && beta >= 0. && gamma >= 0.)
            {
                if (   alpha > 0 || denominators.x() * fa(offscreenPoint) > 0
                    && beta  > 0 || denominators.y() * fb(offscreenPoint) > 0
                    && gamma > 0 || denominators.z() * fc(offscreenPoint) > 0)
                {
                    // Perspective correct interpolation.
                    // see: https://stackoverflow.com/a/24460895/1027706
                    // see: https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/perspective-correct-interpolation-vertex-attributes

                    // Linearly interpolate depth and depth inverse in window space
                    double depthInverse =
                        alpha * aTriangle.a.depthInverse
                        + beta  * aTriangle.b.depthInverse
                        + gamma * aTriangle.c.depthInverse;
                    // TODO Understand why the depth is interpolated without perspective correction?
                    auto z = alpha * aTriangle.a.pos.z()
                           + beta  * aTriangle.b.pos.z()
                           + gamma * aTriangle.c.pos.z();

                    math::Vec<3> perspectiveCoeff = math::Vec<3>{
                        alpha * aTriangle.a.depthInverse,
                        beta  * aTriangle.b.depthInverse,
                        gamma * aTriangle.c.depthInverse
                    } / depthInverse;

                    // Perspective corrected interpolation of varying attributes
                    typename T_vertex::FragmentInterpolated interpolated;
                    // TODO write some generic interpolation code
                    interpolated.color = perspectiveCoeff[0] * aTriangle.a.frag.color
                        + perspectiveCoeff[1] * aTriangle.b.frag.color
                        + perspectiveCoeff[2] * aTriangle.c.frag.color;

                    interpolated.normal_c = perspectiveCoeff[0] * aTriangle.a.frag.normal_c
                        + perspectiveCoeff[1]  * aTriangle.b.frag.normal_c
                        + perspectiveCoeff[2] * aTriangle.c.frag.normal_c;
                    interpolated.normal_c.normalize();

                    interpolated.position_c = perspectiveCoeff[0] * aTriangle.a.frag.position_c
                        + perspectiveCoeff[1]  * aTriangle.b.frag.position_c.as<math::Vec>()
                        + perspectiveCoeff[2] * aTriangle.c.frag.position_c.as<math::Vec>();

                    interpolated.uv = perspectiveCoeff[0] * aTriangle.a.frag.uv
                        + perspectiveCoeff[1] * aTriangle.b.frag.uv.as<math::Vec>()
                        + perspectiveCoeff[2] * aTriangle.c.frag.uv.as<math::Vec>();

                    aFragmentCallback(aRaster, {x, y}, z, interpolated);
                }
            }
            numerators += xIncrements;
        }
        numerators = previousNumerators + yIncrements;
        previousNumerators = numerators;
    }
}


} // namespace focg
} // namespace ad

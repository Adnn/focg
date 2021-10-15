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

template <class T_raster>
void rasterize(const Triangle & aTriangle, T_raster & aRaster)
{
    // Use to assign exactly tangent pixels on adjacent triangles (p 168)
    const HPos offscreenPoint{-1., -1., 0., 1.};

    auto fa = aTriangle.getFa();
    auto fb = aTriangle.getFb();
    auto fc = aTriangle.getFc();

    const double falpha = fa(aTriangle.a);
    const double fbeta  = fb(aTriangle.b);
    const double fgamma = fc(aTriangle.c);

    // Test for degenerate triangle (zero area), which should not be rasterized.
    if (falpha == 0. || fbeta == 0. || fgamma == 0.)
    {
        return; 
    }


    for (auto y = static_cast<int>(std::nearbyint(aTriangle.ymin()));
         y <= static_cast<int>(std::nearbyint(aTriangle.ymax()));
         ++y)
    {
        for (auto x = static_cast<int>(std::nearbyint(aTriangle.xmin()));
             x <= static_cast<int>(std::nearbyint(aTriangle.xmax()));
             ++x)
        {
            double alpha = fa({(double)x, (double)y, 0., 1.}) / falpha;
            double beta  = fb({(double)x, (double)y, 0., 1.}) / fbeta;
            double gamma = fc({(double)x, (double)y, 0., 1.}) / fgamma;

            if (alpha >= 0. && beta >= 0. && gamma >= 0.)
            {
                if (   alpha > 0 || falpha * fa(offscreenPoint) > 0
                    && beta  > 0 || fbeta  * fb(offscreenPoint) > 0
                    && gamma > 0 || fgamma * fc(offscreenPoint) > 0)
                {
                    aRaster.at(x, y) = to_sdr(
                          alpha * aTriangle.a.color
                        + beta  * aTriangle.b.color
                        + gamma * aTriangle.c.color);
                }
            }
        }
    }
}


template <class T_raster>
void defaultFragmentShader(T_raster & aRaster, math::Position<2, int> aScreenPosition, double aDepth, math::sdr::Rgb aColor)
{
    if constexpr (std::is_same_v<T_raster, arte::Image<>>)
    {
        aRaster.at(aScreenPosition.x(), aScreenPosition.y()) = aColor;
    }
    else
    {
        aRaster.color.at(aScreenPosition.x(), aScreenPosition.y()) = aColor;
    }
}


template <class T_raster>
void rasterizeIncremental(const Triangle & aTriangle, T_raster & aRaster)
{
    rasterizeIncremental(aTriangle, aRaster, &defaultFragmentShader<T_raster>);
}


template <class T_raster, class T_fragmentShader>
void rasterizeIncremental(const Triangle & aTriangle, 
                          T_raster & aRaster,
                          const T_fragmentShader & aShader)
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
                    auto z = alpha * aTriangle.a.pos.z()
                           + beta  * aTriangle.b.pos.z()
                           + gamma * aTriangle.c.pos.z();

                    auto color = alpha * aTriangle.a.color
                               + beta  * aTriangle.b.color
                               + gamma * aTriangle.c.color;

                    aShader(aRaster, {x, y}, z, to_sdr(color));
                }
            }
            numerators += xIncrements;
        }
        numerators = previousNumerators + yIncrements;
        previousNumerators = numerators;
    }
}


// Use the math::Barycentric facility, which prevents to implement an incremental version
// No tangent edge assignment implemented.
// No test for degenerate triangles implemented
template <class T_raster>
void rasterizeBis(const Triangle & aTriangle, T_raster & aRaster)
{
    ad::math::Barycentric<double> barycentric{ 
        {aTriangle.a.pos.x(), aTriangle.a.pos.y()},
        {aTriangle.b.pos.x(), aTriangle.b.pos.y()},
        {aTriangle.c.pos.x(), aTriangle.c.pos.y()}
    };

    for (auto y = static_cast<int>(std::nearbyint(aTriangle.ymin()));
         y <= static_cast<int>(std::nearbyint(aTriangle.ymax()));
         ++y)
    {
        for (auto x = static_cast<int>(std::nearbyint(aTriangle.xmin()));
             x <= static_cast<int>(std::nearbyint(aTriangle.xmax()));
             ++x)
        {
            // Shortcut version leads to artifacts on the "alpha edge".
            //auto [alpha, beta, gamma] = barycentric.getCoordinatesShortcut({(double)x, (double)y});
            auto [alpha, beta, gamma] = barycentric.getCoordinates({(double)x, (double)y});
            if (alpha >= 0. && beta >= 0. && gamma >= 0.)
            {
                aRaster.at(x, y) = to_sdr(
                      alpha * aTriangle.a.color
                    + beta  * aTriangle.b.color
                    + gamma * aTriangle.c.color);
            }
        }
    }
}




#if 0 // outline of the incremental optimization proposed in the book
template <class T_raster>
void drawLine(
    Line aLine,
    T_raster & aRaster,
    const math::sdr::Rgb aColor = math::sdr::gWhite)
{
    // FoCG 3rd p 165

    Pos & a = aLine.pointA;
    Pos & b = aLine.pointB;

    auto f = aLine.getImplicitEquation();

    if (a.x() > b.x())
    {
        std::swap(a, b);
    }

    double xb_xa = b.x() - a.x();
    double yb_ya = b.y() - a.y();
    double xa_xb = a.x() - b.x();
    double ya_yb = a.y() - b.y();

    double m = yb_ya / xb_xa;
    // The four cases depending on the slope
    if (m <= -1)
    {
        int x = std::nearbyint(a.x());
        double d = f(a + math::Vec<2>{0.5, 1.});

        for (int y = std::nearbyint(a.y()); y >= std::nearbyint(b.y()); --y)
        {
            aRaster.at(x, y) = aColor;
            if (d < 0)
            {
                x += 1;
                d += -xb_xa + ya_yb;
            }
            else
            {
                d += -xb_xa;
            }
        }
    }
    else if (m <= 1)
    {
        int y = std::nearbyint(a.y());
        double d = f(a + math::Vec<2>{1., 0.5});

        for (int x = std::nearbyint(a.x()); x <= std::nearbyint(b.x()); ++x)
        {
            aRaster.at(x, y) = aColor;
            if (d < 0)
            {
                y += 1;
                d += xb_xa + ya_yb;
            }
            else
            {
                d += ya_yb;
            }
        }
    }
}
#endif


} // namespace focg
} // namespace ad

#pragma once


#include "Line.h"
#include "Triangle.h"

#include <math/Barycentric.h>
#include <math/Color.h>
#include <math/Vector.h>

#include <utility>


namespace ad {
namespace focg {

// TODO color interpolation along the line
/// \note Midpoint algorithgm, FoCG 3rd p 165
template <class T_raster>
void rasterizeLine(
    Line aLine,
    T_raster & aRaster,
    const math::sdr::Rgb aColor = math::sdr::gWhite)
{
    Pos & a = aLine.pointA;
    Pos & b = aLine.pointB;

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
        for (int y = std::nearbyint(a.y()); y >= std::nearbyint(b.y()); --y)
        {
            aRaster.at(x, y) = aColor;
            if (f({x+0.5, y+1.}) < 0)
            {
                ++x;
            }
        }
    }
    else if (m <= 0)
    {
        int y = std::nearbyint(a.y());
        for (int x = std::nearbyint(a.x()); x <= std::nearbyint(b.x()); ++x)
        {
            aRaster.at(x, y) = aColor;
            if (f({x+1., y+0.5}) > 0)
            {
                --y;
            }
        }
    }
    else if (m <= 1)
    {
        int y = std::nearbyint(a.y());
        for (int x = std::nearbyint(a.x()); x <= std::nearbyint(b.x()); ++x)
        {
            aRaster.at(x, y) = aColor;
            if (f({x+1., y+0.5}) < 0)
            {
                ++y;
            }
        }
    }
    else
    {
        int x = std::nearbyint(a.x());
        for (int y = std::nearbyint(a.y()); y <= std::nearbyint(b.y()); ++y)
        {
            aRaster.at(x, y) = aColor;
            if (f({x+0.5, y+1.}) > 0)
            {
                ++x;
            }
        }
    }
}


template <class T_raster>
void rasterize(const Triangle & aTriangle, T_raster & aRaster)
{
    // Use to assign exactly tangent pixels on adjacent triangles (p 168)
    const Pos offscreenPoint{-1., -1.};

    auto fa = aTriangle.getFa();
    auto fb = aTriangle.getFb();
    auto fc = aTriangle.getFc();

    const double falpha = fa(aTriangle.a);
    const double fbeta  = fb(aTriangle.b);
    const double fgamma = fc(aTriangle.c);

    for (auto y = static_cast<int>(std::nearbyint(aTriangle.ymin()));
         y <= static_cast<int>(std::nearbyint(aTriangle.ymax()));
         ++y)
    {
        for (auto x = static_cast<int>(std::nearbyint(aTriangle.xmin()));
             x <= static_cast<int>(std::nearbyint(aTriangle.xmax()));
             ++x)
        {
            double alpha = fa({(double)x, (double)y}) / falpha;
            double beta  = fb({(double)x, (double)y}) / fbeta;
            double gamma = fc({(double)x, (double)y}) / fgamma;

            if (alpha >= 0. && beta >= 0. && gamma >= 0.)
            {
                if (   alpha > 0 || falpha * fa(offscreenPoint) > 0
                    && beta  > 0 || fbeta  * fb(offscreenPoint) > 0
                    && gamma > 0 || fgamma * fc(offscreenPoint) > 0)
                {
                    aRaster.at(x, y) =
                          alpha * aTriangle.a.color
                        + beta  * aTriangle.b.color
                        + gamma * aTriangle.c.color
                        ;
                }
            }
        }
    }
}


template <class T_raster>
void rasterizeIncremental(const Triangle & aTriangle, T_raster & aRaster)
{
    // Use to assign exactly tangent pixels on adjacent triangles (p 168)
    const Pos offscreenPoint{-1., -1.};

    // Setup
    auto fa = aTriangle.getFa();
    auto fb = aTriangle.getFb();
    auto fc = aTriangle.getFc();

    const double xmin = aTriangle.xmin();
    const double ymin = aTriangle.ymin();

    math::Vec<3, double> previousNumerators{
        fa({xmin, ymin}),
        fb({xmin, ymin}),
        fc({xmin, ymin}) 
    };

    const math::Vec<3, double> denominators{
        fa(aTriangle.a),
        fb(aTriangle.b),
        fc(aTriangle.c)
    };

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
    for (auto y = static_cast<int>(std::nearbyint(ymin));
         y <= static_cast<int>(std::nearbyint(aTriangle.ymax()));
         ++y)
    {
        for (auto x = static_cast<int>(std::nearbyint(xmin));
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
                    aRaster.at(x, y) =
                          alpha * aTriangle.a.color
                        + beta  * aTriangle.b.color
                        + gamma * aTriangle.c.color
                        ;
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
template <class T_raster>
void rasterizeBis(const Triangle & aTriangle, T_raster & aRaster)
{
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
                aRaster.at(x, y) =
                      alpha * aTriangle.a.color
                    + beta  * aTriangle.b.color
                    + gamma * aTriangle.c.color
                    ;
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

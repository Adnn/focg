#pragma once


#include "Line.h"

#include <math/Color.h>
#include <math/Vector.h>

#include <utility>


namespace ad {
namespace focg {

template <class T_raster>
void rasterizeLine(
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

    double m = (b.y() - a.y()) / (b.x() - a.x());
    // The four cases depending on the slope
    if (m <= -1)
    {
        int x = std::round(a.x());
        for (int y = std::round(a.y()); y >= std::round(b.y()); --y)
        {
            aRaster.at(x, (aRaster.height()-1) - y) = aColor;
            if (f({x+0.5, y+1.}) < 0)
            {
                ++x;
            }
        }
    }
    else if (m <= 0)
    {
        int y = std::round(a.y());
        for (int x = std::round(a.x()); x <= std::round(b.x()); ++x)
        {
            aRaster.at(x, (aRaster.height()-1) - y) = aColor;
            if (f({x+1., y+0.5}) > 0)
            {
                --y;
            }
        }
    }
    else if (m <= 1)
    {
        int y = std::round(a.y());
        for (int x = std::round(a.x()); x <= std::round(b.x()); ++x)
        {
            aRaster.at(x, (aRaster.height()-1) - y) = aColor;
            if (f({x+1., y+0.5}) < 0)
            {
                ++y;
            }
        }
    }
    else
    {
        int x = std::round(a.x());
        for (int y = std::round(a.y()); y <= std::round(b.y()); ++y)
        {
            aRaster.at(x, (aRaster.height()-1) - y) = aColor;
            if (f({x+0.5, y+1.}) > 0)
            {
                ++x;
            }
        }
    }
}



#if 0 // outline of the optimization proposed in the book
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
        int x = std::round(a.x());
        double d = f(a + math::Vec<2>{0.5, 1.});

        for (int y = std::round(a.y()); y >= std::round(b.y()); --y)
        {
            aRaster.at(x, (aRaster.height()-1) - y) = aColor;
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
        int y = std::round(a.y());
        double d = f(a + math::Vec<2>{1., 0.5});

        for (int x = std::round(a.x()); x <= std::round(b.x()); ++x)
        {
            aRaster.at(x, (aRaster.height()-1) - y) = aColor;
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

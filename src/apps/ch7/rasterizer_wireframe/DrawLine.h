#pragma once


#include "Line.h"


#include <math/Color.h>
#include <math/Vector.h>


namespace ad {
namespace focg {


template <class T_raster>
void drawLine(math::Position<2, int> a, const math::Position<2, int> b,
              T_raster & aRaster, const math::sdr::Rgb aColor = math::sdr::gWhite)
{
    // see: https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    int dx =  std::abs(b.x() - a.x());
    int sx = a.x() < b.x() ? 1 : -1;
    int dy = - std::abs(b.y() - a.y());
    int sy = a.y() < b.y() ? 1 : -1;
    int err = dx + dy;  /* error value e_xy */

    const math::Rectangle<int> imageRect{{0, 0}, aRaster.dimensions() - math::Size<2, int>{1, 1}};
    while (true)   /* loop */
    {
        if (imageRect.contains(a))
        {
            aRaster.at(a.x(), aRaster.height() - a.y() - 1) = aColor;
        }
        if (a.x() == b.x() && a.y() == b.y())
        {
            break;
        }

        int e2 = 2 * err;
        if (e2 >= dy) /* e_xy+e_x > 0 */
        {
            err += dy;
            a.x() += sx;
        }
        if (e2 <= dx) /* e_xy+e_y < 0 */
        {
            err += dx;
            a.y() += sy;
        }
    }
}


template <class T_raster>
void drawLine(const Line & aLine, T_raster && aRaster, const math::sdr::Rgb aColor = math::sdr::gWhite)
{
    drawLine({(int)aLine.pointA.x(), (int)aLine.pointA.y()},
             {(int)aLine.pointB.x(), (int)aLine.pointB.y()},
             std::forward<T_raster>(aRaster),
             aColor);
}


} // namespace focg
} // namespace ad

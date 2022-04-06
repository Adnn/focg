#pragma once


#include "Line.h"

#include <math/Color.h>
#include <math/Rectangle.h>
#include <math/Vector.h>

#include <string>


namespace ad {
namespace focg {

using HPos = math::Position<4>;
using Rectangle = math::Rectangle<double>;


struct Vertex
{
    /*implicit*/ operator HPos & ()
    {
        return pos;
    }

    /*implicit*/ operator HPos () const
    {
        return pos;
    }

    HPos pos;
    math::hdr::Rgb_d color;
};


struct Triangle
{
    Vertex a;
    Vertex b;
    Vertex c;

    Vertex & at(std::size_t aIndex)
    {
        switch (aIndex)
        {
        default:
            throw std::domain_error{"Invalid indice: " + std::to_string(aIndex)};
        case 0:
            return a;
        case 1:
            return b;
        case 2:
            return c;
        }
    }

    double xmin() const
    {
        return std::min({a.pos.x(), b.pos.x(), c.pos.x()});
    }

    double xmax() const
    {
        return std::max({a.pos.x(), b.pos.x(), c.pos.x()});
    }

    double ymin() const
    {
        return std::min({a.pos.y(), b.pos.y(), c.pos.y()});
    }

    double ymax() const
    {
        return std::max({a.pos.y(), b.pos.y(), c.pos.y()});
    }

    Line getLineA() const
    {
        return Line{b.pos, c.pos};
    }

    Line getLineB() const
    {
        return Line{c.pos, a.pos};
    }

    Line getLineC() const
    {
        return Line{a.pos, b.pos};
    }

    auto getFa() const
    {
        return getLineA().getImplicitEquation();
    }

    auto getFb() const
    {
        return getLineB().getImplicitEquation();
    }

    auto getFc() const
    {
        return getLineC().getImplicitEquation();
    }

    Triangle & transform(const math::AffineMatrix<4> & aTransformation)
    {
        a.pos *= aTransformation;
        b.pos *= aTransformation;
        c.pos *= aTransformation;
        return *this;
    }

    Triangle & perspectiveDivide()
    {
        a.pos /= a.pos.w();
        b.pos /= b.pos.w();
        c.pos /= c.pos.w();
        return *this;
    }
};


} // namespace focg
} // namespace ad

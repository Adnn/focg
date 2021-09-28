#pragma once


#include "Line.h"

#include <math/Color.h>
#include <math/Rectangle.h>
#include <math/Vector.h>


namespace ad {
namespace focg {

using Pos = math::Position<2>;
using Rectangle = math::Rectangle<double>;


struct Vertex
{
    /*implicit*/ operator Pos & () 
    {
        return pos;
    }

    /*implicit*/ operator Pos () const
    {
        return pos;
    }

    Pos pos;
    math::sdr::Rgb color;
};


struct Triangle
{
    Vertex a;
    Vertex b;
    Vertex c;

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
};


} // namespace focg
} // namespace ad

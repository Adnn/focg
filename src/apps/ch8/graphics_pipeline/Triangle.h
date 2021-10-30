#pragma once


#include "Line.h"

#include <math/Color.h>
#include <math/Rectangle.h>
#include <math/Vector.h>

#include <string>


namespace ad {
namespace focg {

using HPos = math::Position<4>;
using HVec = math::Vec<4>;
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
    math::hdr::Rgb color;
};


template <class T_vertex>
struct Triangle_base
{
    T_vertex a;
    T_vertex b;
    T_vertex c;

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

    Triangle_base & transform(const math::AffineMatrix<4> & aTransformation)
    {
        a.pos *= aTransformation;
        b.pos *= aTransformation;
        c.pos *= aTransformation;
        return *this;
    }

    Triangle_base & perspectiveDivide()
    {
        a.pos /= a.pos.w();
        b.pos /= b.pos.w();
        c.pos /= c.pos.w();
        return *this;
    }
};


using Triangle = Triangle_base<Vertex>;


} // namespace focg
} // namespace ad

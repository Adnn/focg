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
using TextureCoordinates = math::Position<2>;
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
    HVec normal{0., 0., 0., 0.}; // must have defaults, when creating the vertex set in the obj loader
    TextureCoordinates uv{0., 0.}; // must have defaults, when creating the vertex set in the obj loader

    // TODO Ideally, this should not be hosted within the vertex
    // Then, it does not neet to be default constructible
    struct FragmentInterpolated
    {
        math::hdr::Rgb color = math::hdr::gRed;
        HPos position_c{0., 0., 0., 1.};
        HVec normal_c{0., 0., 0., 0.};
        TextureCoordinates uv{0., 0.};
    };

    FragmentInterpolated frag;
};


template <class T_vertex>
struct Triangle
{
    T_vertex a;
    T_vertex b;
    T_vertex c;

    T_vertex & at(std::size_t aIndex)
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

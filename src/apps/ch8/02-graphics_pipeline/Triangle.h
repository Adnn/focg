#pragma once


#include "Line.h"

#include <math/Color.h>
#include <math/Rectangle.h>
#include <math/Vector.h>

#include <string>

#include <cassert>


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

    void perspectiveDivide()
    {
        // This depth inverse is stored to implement perspective correct interpolation.
        // See: https://stackoverflow.com/a/24460895/1027706
        pos.x() /= pos.w();
        pos.y() /= pos.w();
        pos.z() /= pos.w();
        // Important: this breaks the later viewports transform, just store it separately atm
        //pos.w() = 1. / pos.w();
        depthInverse = 1. / pos.w();
        pos.w() = 1.;
    }

    HPos pos;
    math::hdr::Rgb_d color = math::hdr::gRed<>;
    double depthInverse = 0.;
    HVec normal{0., 0., 0., 0.}; // must have defaults, when creating the vertex set in the obj loader
    TextureCoordinates uv{0., 0.}; // must have defaults, when creating the vertex set in the obj loader

    // TODO Ideally, this should not be hosted within the vertex
    // Then, it does not neet to be default constructible
    struct FragmentInterpolated
    {
        math::hdr::Rgb_d color = math::hdr::gRed<>;
        HPos position_c{0., 0., 0., 1.};
        HVec normal_c{0., 0., 0., 0.};
        TextureCoordinates uv{0., 0.};
    };

    FragmentInterpolated frag;
};


template <class T_value>
struct Interpolant
{
    double weight;
    T_value value;
};


// Note: This syntax with a constrained accessor type is turned out very impractical:
// Invocation required to specify all template parameters, and the lambda had to specify its return type.
//template <class T_value, class T_accessed = T_value>
//T_accessed interpolateLinear(std::initializer_list<Interpolant<T_value>> aInterpolants, 
//                             const T_accessed &(* aAccessor)(const T_value &) = [](const T_value & aValue) -> const T_accessed &{return aValue;})

// Note: This is almost equivalently constrained, as the return type will only substitute if T_accessor
// has an operator() taking a const T_value reference.
template <class T_value, class T_accessor>
decltype(std::declval<T_accessor &>()(std::declval<const T_value &>())) 
interpolateLinear(std::initializer_list<Interpolant<T_value>> aInterpolants, T_accessor aAccessor)
{
    assert(aInterpolants.size() > 0);
    auto accum = aInterpolants.begin()->weight * aAccessor(aInterpolants.begin()->value);
    for (auto it = aInterpolants.begin() + 1; it != aInterpolants.end(); ++it)
    {
        if constexpr(math::is_position_v<decltype(accum)>) 
        {
            accum += it->weight * aAccessor(it->value).template as<math::Vec>(); 
        }
        else
        {
            accum += it->weight * aAccessor(it->value); 
        }
    }
    return accum;
}


Vertex::FragmentInterpolated interpolateLinear(std::initializer_list<Interpolant<Vertex::FragmentInterpolated>> aInterpolants)
{
    return {
        interpolateLinear(aInterpolants, [](const Vertex::FragmentInterpolated & frag){return frag.color;}),
        interpolateLinear(aInterpolants, [](const Vertex::FragmentInterpolated & frag){return frag.position_c;}),
        interpolateLinear(aInterpolants, [](const Vertex::FragmentInterpolated & frag){return frag.normal_c;}).normalize(),
        interpolateLinear(aInterpolants, [](const Vertex::FragmentInterpolated & frag){return frag.uv;}),
    };
}


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

    math::Vec<3> facetNormal() const
    {
        HVec ab{b.pos - a.pos};
        HVec ac{c.pos - a.pos};
        return math::Vec<3>{ab.x(), ab.y(), ab.z()}.cross(math::Vec<3>{ac.x(), ac.y(), ac.z()});
    }

    bool isFacingFront() const
    {
        HVec ab{b.pos - a.pos};
        HVec ac{c.pos - a.pos};
        // Only the Z component of the cross product
        return (ab.x() * ac.y() - ab.y() * ac.x()) > 0;
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
        a.perspectiveDivide();
        b.perspectiveDivide();
        c.perspectiveDivide();
        return *this;
    }
};



} // namespace focg
} // namespace ad

#pragma once


#include <math/Homogeneous.h>
#include <math/Matrix.h>
#include <math/Vector.h>

#include <functional>


namespace ad {
namespace focg {


using Pos  = math::Position<2>;
using HPos = math::Position<4>;


// TODO templatize on T_vertex
struct Line
{
    Line(HPos a, HPos b) :
        pointA{std::move(a)},
        pointB{std::move(b)}
    {}

    Line(Pos a, Pos b) :
        pointA{a, 0., 1.},
        pointB{b, 0., 1.}
    {}

    double getEquationFactorX() const
    {
        return pointA.y() - pointB.y();
    }

    double getEquationFactorY() const
    {
        return pointB.x() - pointA.x();
    }

    auto getImplicitEquation() const
    {
        // simple copy by value would capture `this`
        // and I would suspect this means it will try to access the values
        // each time (hoping a copy allows the compiler to precompute some operations).
        return [pointA=this->pointA, pointB=this->pointB](HPos point)
        {
            return (pointA.y() - pointB.y()) * point.x() 
                + (pointB.x() - pointA.x()) * point.y()
                + pointA.x() * pointB.y()
                - pointB.x() * pointA.y()
                ;
        };
    }

    HPos pointA;
    HPos pointB;
};


} // namespace focg
} // namespace ad

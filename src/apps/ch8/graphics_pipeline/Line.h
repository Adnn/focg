#pragma once


#include <math/Homogeneous.h>
#include <math/Matrix.h>
#include <math/Vector.h>

#include <functional>


namespace ad {
namespace focg {


using Pos = math::Position<2>;


struct Line
{
    Line(Pos a, Pos b) :
        pointA{std::move(a)},
        pointB{std::move(b)}
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
        return [pointA=this->pointA, pointB=this->pointB](Pos point)
        {
            return (pointA.y() - pointB.y()) * point.x() 
                + (pointB.x() - pointA.x()) * point.y()
                + pointA.x() * pointB.y()
                - pointB.x() * pointA.y()
                ;
        };
    }

    Pos pointA;
    Pos pointB;
};


} // namespace focg
} // namespace ad

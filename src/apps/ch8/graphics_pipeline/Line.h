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

    std::function<Pos::value_type(Pos)> getImplicitEquation() const
    {
        return [=](Pos point)
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

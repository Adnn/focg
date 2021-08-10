#pragma once


#include <math/Homogeneous.h>
#include <math/Matrix.h>
#include <math/Vector.h>


namespace ad {
namespace focg {


struct Line
{
    Line(math::Position<4> a, math::Position<4> b) :
        pointA{std::move(a)},
        pointB{std::move(b)}
    {}

    Line(math::Position<3> a, math::Position<3> b) :
        pointA{math::homogeneous::makePosition(a)},
        pointB{math::homogeneous::makePosition(b)}
    {}

    math::Position<4> pointA;
    math::Position<4> pointB;
};


// Will accept any Matrix or derived (as AffineMatrix)
Line operator *(const Line & aLine, math::Matrix<4, 4, double> & aTransformation)
{
    return {math::homogeneous::normalize(aLine.pointA * aTransformation),
            math::homogeneous::normalize(aLine.pointB * aTransformation)};
}

} // namespace focg
} // namespace ad

#pragma once


#include "Line.h"

#include <math/Box.h>
#include <math/Homogeneous.h>

#include <optional>


namespace ad {
namespace focg {


using HPos = math::Position<4>;
using HVec = math::Vec<4>;


struct ViewVolume;
using Evaluator = double(*)(HPos, const ViewVolume &);


struct ViewVolume
{
    struct Plane
    {
        Plane(Evaluator aEvaluator, HVec aNormal, HPos aPointInPlane) :
            f{aEvaluator},
            n{std::move(aNormal)},
            d{-(aNormal.as<math::Position>().dot(aPointInPlane))}
        {}

        double solveForT(HPos a, HPos b) const
        {
            return (n.as<math::Position>().dot(a) + d) 
                   / n.dot(a - b);
        }

        Evaluator f;         
        HVec n;
        double d;
    };

    ViewVolume(math::Box<double> aBox) :
        l{aBox.xMin()},
        r{aBox.xMax()},
        t{aBox.yMax()},
        b{aBox.yMin()},
        n{aBox.zMax()},
        f{aBox.zMin()},
        planes {
            Plane{ // Left
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return -aPos.x() + aVolume.l * aPos.w(); },
                HVec{-1., 0., 0., 0.},
                math::homogeneous::makePosition(aBox.bottomLeftFront()),
            },
            Plane{ // Right
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return aPos.x() - aVolume.r * aPos.w(); },
                HVec{1., 0., 0., 0.},
                math::homogeneous::makePosition(aBox.topRightBack()),
            },
            Plane{ // Bottom
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return -aPos.y() + aVolume.b * aPos.w(); },
                HVec{0., 1., 0., 0.},
                math::homogeneous::makePosition(aBox.bottomLeftFront()),
            },
            Plane{ // Top
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return aPos.y() - aVolume.t * aPos.w(); },
                HVec{0., -1., 0., 0.},
                math::homogeneous::makePosition(aBox.topRightBack()),
            },
            //Plane{ // Front
            //    [](HPos aPos, const ViewVolume & aVolume) -> double
            //    { return -aPos.z() + aVolume.n * aPos.w(); },
            //    HVec{0., 0., -1., 0.},
            //    math::homogeneous::makePosition(aBox.bottomLeftFront()),
            //},
            //Plane{ // Back
            //    [](HPos aPos, const ViewVolume & aVolume) -> double
            //    { return aPos.z() - aVolume.f * aPos.w(); },
            //    HVec{0., 0., 1., 0.},
            //    math::homogeneous::makePosition(aBox.topRightBack()),
            //},
        }
    {}

    double evaluate(std::size_t aPlaneId, HPos aPosition) const
    {
        return planes[aPlaneId].f(aPosition, *this);
    }

    double solveForT(std::size_t aPlaneId, HPos a, HPos b) const
    {
        return planes[aPlaneId].solveForT(a, b);
    }

    static constexpr std::size_t gPlanesCount = 4;
    double l, r, t, b, n, f;
    std::array<Plane, gPlanesCount> planes;
};


//
// Lines
//
inline std::optional<Line> clip_impl(HPos a, HPos b,
                                     const ViewVolume & aVolume,
                                     std::size_t aStartingPlane)
{
    for (std::size_t planeId = aStartingPlane; planeId != ViewVolume::gPlanesCount; ++planeId)
    {
        double fa = aVolume.evaluate(planeId, a);
        double fb = aVolume.evaluate(planeId, b);

        if (fa <= 0. && fb <= 0.) // both inside
        {
            // go on to next iteration
        }
        else if (fa * fb > 0.) // same side, but outside
        {
            return {};
        }
        else // one point on each side -> the line crosses the plane
        {
            double t = aVolume.solveForT(planeId, a, b);
            if (fb > 0) // b is on the out side of the plane
            {
                return clip_impl(a, a + t * (b - a), aVolume, planeId + 1);
            }
            else // a is outside
            {
                return clip_impl(a + t * (b - a), b, aVolume, planeId + 1);
            }
        }
    }
    return Line{a, b};
}


inline std::optional<Line> clip(Line aLine, const ViewVolume & aVolume)
{
    return clip_impl(aLine.pointA, aLine.pointB, aVolume, 0);
}


} // namespace focg
} // namespace ad

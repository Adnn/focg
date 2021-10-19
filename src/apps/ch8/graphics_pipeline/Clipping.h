#pragma once


#include "Line.h"
#include "Triangle.h"

#include <math/Box.h>
#include <math/Homogeneous.h>
#include <math/Interpolation.h>

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
            Plane{ // Front
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return aPos.z() - aVolume.n * aPos.w(); },
                HVec{0., 0., -1., 0.},
                math::homogeneous::makePosition(aBox.bottomLeftFront()),
            },
            Plane{ // Back
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return -aPos.z() + aVolume.f * aPos.w(); },
                HVec{0., 0., 1., 0.},
                math::homogeneous::makePosition(aBox.topRightBack()),
            },
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

    static constexpr std::size_t gPlanesCount = 6;
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


//
// Triangles
//
template <class T_insertIterator>
inline void clip_impl(const Triangle & aTriangle,
                      T_insertIterator & aInserter,     
                      const ViewVolume & aVolume,
                      std::size_t aStartingPlane)
{
    for (std::size_t planeId = aStartingPlane; planeId != ViewVolume::gPlanesCount; ++planeId)
    {
        double fa = aVolume.evaluate(planeId, aTriangle.a);
        double fb = aVolume.evaluate(planeId, aTriangle.b);
        double fc = aVolume.evaluate(planeId, aTriangle.c);

        if (fa < 0. && fb < 0. && fc < 0.) // all outside
        {
            return; // no insertion taking place
        }
        else if (fa < 0. || fb < 0. || fc < 0) // points are on different sides 
                                               // (at least one outside, but not all)
        {
            Vertex a = aTriangle.a;
            Vertex b = aTriangle.b;
            Vertex c = aTriangle.c;

            // Reorganize the triangle edges to make sure that:
            // * c is on one side
            // * a & b are both on the other side
            // see FoCG 3rd p296

            if (fa * fc >= 0) // b is alone on the other side
            {
                // rotate backward (a=c, b=a, c=b)
                std::swap(fb, fc);
                std::swap(b, c);
                std::swap(fa, fb);
                std::swap(a, b);
            }
            else if (fb * fc >= 0) // a is alone on the otherside
            {
                // rotate forward (a=b, b=c, c=a)
                std::swap(fa, fc);
                std::swap(a, c);
                std::swap(fa, fb);
                std::swap(a, b);
            }

            double t_ac = aVolume.solveForT(planeId, a, c);
            double t_bc = aVolume.solveForT(planeId, b, c);

            // Define intersection position, and interpolate colors
            Vertex vertexAC{a.pos + t_ac * (c.pos - a.pos), math::lerp(a.color, c.color, t_ac)};
            Vertex vertexBC{b.pos + t_bc * (c.pos - b.pos), math::lerp(b.color, c.color, t_bc)};

            if (fc > 0) // c is on the in side of the plane, spawn a single triangle
            {
                clip_impl({vertexAC, vertexBC, c}, aInserter, aVolume, aStartingPlane + 1);
                return;
            }
            else // c is outside, spawn two triangles
            {
                clip_impl({a, b,        vertexAC}, aInserter, aVolume, aStartingPlane + 1);
                clip_impl({b, vertexBC, vertexAC}, aInserter, aVolume, aStartingPlane + 1);
                return;
            }
        }
    }
    aInserter = aTriangle;
}


inline std::vector<Triangle> clip(const Triangle & aTriangle, const ViewVolume & aVolume)
{
    std::vector<Triangle> result;
    clip_impl(aTriangle, std::back_inserter(result), aVolume, 0);
    return result;
}

} // namespace focg
} // namespace ad

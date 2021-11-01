#pragma once


#include "Line.h"
#include "Triangle.h"

#include <math/Box.h>
#include <math/Homogeneous.h>
#include <math/Interpolation.h>

#include <optional>

#include <cassert>


namespace ad {
namespace focg {


using HPos = math::Position<4>;
using HVec = math::Vec<4>;


struct ViewVolume;
using Evaluator = double(*)(HPos, const ViewVolume &);
using Solver = double(*)(HPos, HPos);


struct ViewVolume
{
    // Clipping in homogenous space
    // see: https://fabiensanglard.net/polygon_codec/
    // Note: For the equation of the solver in clipping space,
    // see: https://fabiensanglard.net/polygon_codec/clippingdocument/p245-blinn.pdf (CLIPPING USING HOMOGENEOUS COORDINATES, Blinn)
    // see: https://fabiensanglard.net/polygon_codec/clippingdocument/Clipping.pdf (Clipping, Kenneth I. Joy)
    struct Plane
    {
        Evaluator f;         
        Solver solver;
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
                [](HPos a, HPos b) -> double
                { return (a.w() + a.x()) / ( (a.w() + a.x()) - (b.w() + b.x())); },
            },
            Plane{ // Right
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return aPos.x() - aVolume.r * aPos.w(); },
                [](HPos a, HPos b) -> double
                { return (a.w() - a.x()) / ( (a.w() - a.x()) - (b.w() - b.x())); },
            },
            Plane{ // Bottom
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return -aPos.y() + aVolume.b * aPos.w(); },
                [](HPos a, HPos b) -> double
                { return (a.w() + a.y()) / ( (a.w() + a.y()) - (b.w() + b.y())); },
            },
            Plane{ // Top
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return aPos.y() - aVolume.t * aPos.w(); },
                [](HPos a, HPos b) -> double
                { return (a.w() - a.y()) / ( (a.w() - a.y()) - (b.w() - b.y())); },
            },
            Plane{ // Front
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return aPos.z() - aVolume.n * aPos.w(); },
                [](HPos a, HPos b) -> double
                { return (a.w() - a.z()) / ( (a.w() - a.z()) - (b.w() - b.z())); },
            },
            Plane{ // Back
                [](HPos aPos, const ViewVolume & aVolume) -> double
                { return -aPos.z() + aVolume.f * aPos.w(); },
                [](HPos a, HPos b) -> double
                { return (a.w() + a.z()) / ( (a.w() + a.z()) - (b.w() + b.z())); },
            },
        }
    {}

    double evaluate(std::size_t aPlaneId, HPos aPosition) const
    {
        return planes[aPlaneId].f(aPosition, *this);
    }

    double solveForT(std::size_t aPlaneId, HPos a, HPos b) const
    {
        return planes[aPlaneId].solver(a, b);
    }

    static constexpr std::size_t gPlanesCount = 6;
    double l, r, t, b, n, f;
    std::array<Plane, gPlanesCount> planes;
};


/// \brief Returns true if both evaluations place the point on the same side of the plane
///
/// \important Both points are on the same side if:
/// * both their evaluations are non-null and of the same sign 
/// * or both are either negative or zero.
bool onSameSide(double aLhs, double aRhs)
{
    double sign = aLhs * aRhs;
    return (sign > 0.) || (sign == 0. && (aLhs + aRhs) <= 0.);
}

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
template <class T_vertex, class T_insertIterator>
inline void clip_impl(const Triangle<T_vertex> & aTriangle,
                      T_insertIterator & aInserter,     
                      const ViewVolume & aVolume,
                      std::size_t aStartingPlane)
{
    for (std::size_t planeId = aStartingPlane; planeId != ViewVolume::gPlanesCount; ++planeId)
    {
        double fa = aVolume.evaluate(planeId, aTriangle.a);
        double fb = aVolume.evaluate(planeId, aTriangle.b);
        double fc = aVolume.evaluate(planeId, aTriangle.c);

        if (fa > 0. && fb > 0. && fc > 0.) // all outside
        {
            return; // no insertion taking place
        }
        else if (fa > 0. || fb > 0. || fc > 0) // points are on different sides 
                                               // (at least one outside, but not all)
        {
            T_vertex a = aTriangle.a;
            T_vertex b = aTriangle.b;
            T_vertex c = aTriangle.c;

            // Reorganize the triangle edges to make sure that:
            // * c is on one side
            // * a & b are both on the other side
            // see FoCG 3rd p296

            if (onSameSide(fa, fc)) // b is alone on the other side
            {
                // rotate backward (a=c, b=a, c=b)
                std::swap(fb, fc);
                std::swap(b, c);
                std::swap(fa, fb);
                std::swap(a, b);
            }
            else if (onSameSide(fb, fc)) // a is alone on the otherside
            {
                // rotate forward (a=b, b=c, c=a)
                std::swap(fa, fc);
                std::swap(a, c);
                std::swap(fa, fb);
                std::swap(a, b);
            }

            // NOTE: After swapping, C must be alone on one side of the clipping plane.
            assert(( (void)"Point C must be swapped alone on one side of the plane.",
                     (fc <= 0. && fa > 0. && fb > 0.) || (fc > 0. && fa <= 0. && fb <= 0.) )
            );

            double t_ac = aVolume.solveForT(planeId, a, c);
            double t_bc = aVolume.solveForT(planeId, b, c);

            // NOTE: The solution must strictly be on the line segment.
            assert( 0.0 <= t_ac && t_ac <= 1.0 && 0.0 <= t_bc && t_bc <= 1.0 );

            // Define intersection position
            T_vertex vertexAC{a.pos + t_ac * (c.pos - a.pos)};
            T_vertex vertexBC{b.pos + t_bc * (c.pos - b.pos)};

            // Interpolate fragment varying attributes
            vertexAC.frag = interpolateLinear({ {(1 - t_ac), a.frag}, {t_ac, c.frag} });
            vertexBC.frag = interpolateLinear({ {(1 - t_bc), b.frag}, {t_bc, c.frag} });

            // NOTE evaluation == 0 is also considering the point on the in side.
            if (fc <= 0) // c is on the *in* side of the plane, spawn a single triangle
            {
                clip_impl<T_vertex>({vertexAC, vertexBC, c}, aInserter, aVolume, aStartingPlane + 1);
                return;
            }
            else // c is outside, spawn two triangles
            {
                clip_impl<T_vertex>({a, b,        vertexAC}, aInserter, aVolume, aStartingPlane + 1);
                clip_impl<T_vertex>({b, vertexBC, vertexAC}, aInserter, aVolume, aStartingPlane + 1);
                return;
            }
        }
    }
    aInserter = aTriangle;
}


template <class T_vertex>
inline std::vector<Triangle<T_vertex>> clip(const Triangle<T_vertex> & aTriangle, const ViewVolume & aVolume)
{
    std::vector<Triangle<T_vertex>> result;
    clip_impl(aTriangle, std::back_inserter(result), aVolume, 0);
    return result;
}

} // namespace focg
} // namespace ad

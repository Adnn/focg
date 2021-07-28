#pragma once

#include <math/Vector.h>

namespace ad {
namespace focg {


struct Ray
{
    math::Position<3> origin;
    // IMPORTANT: the book does not seem to make it unit length (notably using d.dot(d) in the intersection formula).
    // Yet, it would seem better, notably for crowded scenes.
    math::Vec<3> direction;

    math::Position<3> operator()(double t) const
    {
        return origin + t * direction;
    }
};


} // namespace focg
} // namespace ad

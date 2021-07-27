#pragma once

#include <math/Vector.h>


namespace ad {
namespace focg {

struct Interval
{
    double t0{0};
    double t1{std::numeric_limits<double>::max()};

    bool rightTrim(double t)
    {
        if (t < t1)
        {
            t1 = t;
            return true;
        }
        return false;
    }
};


struct Hit
{
    double t;
    math::UnitVec<3> normal;
    // Material
};

} // namespace focg
} // namespace ad

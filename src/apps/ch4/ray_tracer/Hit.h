#pragma once

#include "Material.h"

#include <math/Vector.h>

#include <memory>


namespace ad {
namespace focg {

struct Interval
{
    double t0{0};
    double t1{std::numeric_limits<double>::max()};

    bool trimRight(double t)
    {
        if (t>= t0 && t < t1)
        {
            t1 = t;
            return true;
        }
        return false;
    }

    static constexpr double gEpsilon{0.001};
};


struct Hit
{
    double t;
    math::Position<3> position;
    math::UnitVec<3> normal;
    std::shared_ptr<Material> material;
};

} // namespace focg
} // namespace ad

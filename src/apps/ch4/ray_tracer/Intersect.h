#pragma once


#include "Hit.h"
#include "Ray.h"
#include "Surfaces.h"


namespace ad {
namespace focg {


Hit make_hit(const Ray & aRay, const Sphere & aSphere, double t)
{
    math::Position<3> hitPoint = aRay(t);
    return Hit{
        t,
        hitPoint,
        math::UnitVec<3>::make_fromUnitLength((aRay(t) - aSphere.center) / aSphere.radius),
        aSphere.material
    };
}

inline std::optional<Hit> intersect(const Ray & aRay, const Sphere & aSphere, Interval aInterval)
{
    auto d = aRay.direction;
    auto e = aRay.origin;
    auto c = aSphere.center;
    auto emc = e-c;

    auto discriminant = std::pow(d.dot(emc), 2)
                        - d.dot(d) * (emc.dot(emc) - std::pow(aSphere.radius, 2));

    if (discriminant == 0)
    {
        auto t = - d.dot(emc) / d.dot(d);
        if (aInterval.rightTrim(t))
        {
            return make_hit(aRay, aSphere, t);
        }
    }
    else if (discriminant > 0)
    {
        auto ta = (- d.dot(emc) - std::sqrt(discriminant))/ d.dot(d);
        auto tb = (- d.dot(emc) + std::sqrt(discriminant))/ d.dot(d);

        // Only hit on tmin (i.e. no hit if the camera is inside the sphere)
        if (auto tmin = std::min(ta, tb); aInterval.rightTrim(tmin))
        {
            return make_hit(aRay, aSphere, tmin);
        }
    }

    // Fall through means no hit
    return {};
}


} // namespace focg
} // namespace ad

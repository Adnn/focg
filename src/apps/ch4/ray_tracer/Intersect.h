#pragma once


#include "Hit.h"
#include "Ray.h"
#include "Surfaces.h"


namespace ad {
namespace focg {


inline Hit make_hit(const Ray & aRay, const Sphere & aSphere, double t)
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
        if (aInterval.trimRight(t))
        {
            return make_hit(aRay, aSphere, t);
        }
    }
    else if (discriminant > 0)
    {
        auto ta = (- d.dot(emc) - std::sqrt(discriminant))/ d.dot(d);
        auto tb = (- d.dot(emc) + std::sqrt(discriminant))/ d.dot(d);

        // Only hit on tmin (i.e. no hit if the camera is inside the sphere)
        if (auto tmin = std::min(ta, tb); aInterval.trimRight(tmin))
        {
            return make_hit(aRay, aSphere, tmin);
        }
    }

    // Fall through means no hit
    return {};
}


inline Hit make_hit(const Ray & aRay, const Triangle & aTriangle, double t)
{
    math::Position<3> hitPoint = aRay(t);
    return Hit{
        t,
        hitPoint,
        aTriangle.getNormal(),
        aTriangle.material
    };
}


inline std::optional<Hit> intersect(const Ray & aRay, const Triangle & aTriangle, Interval aInterval)
{
    // FOCG 3rd: p79
    auto a = aTriangle.a.x() - aTriangle.b.x();
    auto b = aTriangle.a.y() - aTriangle.b.y();
    auto c = aTriangle.a.z() - aTriangle.b.z();

    auto d = aTriangle.a.x() - aTriangle.c.x();
    auto e = aTriangle.a.y() - aTriangle.c.y();
    auto f = aTriangle.a.z() - aTriangle.c.z();

    auto g = aRay.direction.x();
    auto h = aRay.direction.y();
    auto i = aRay.direction.z();

    auto j = aTriangle.a.x() - aRay.origin.x();
    auto k = aTriangle.a.y() - aRay.origin.y();
    auto l = aTriangle.a.z() - aRay.origin.z();

    auto ei_m_hf = e*i - h*f;
    auto gf_m_di = g*f - d*i;
    auto dh_m_eg = d*h - e*g;

    auto M = a * ei_m_hf + b * gf_m_di + c * dh_m_eg;

    if (M == 0)
    {
        return {};
    }

    auto t = -(f * (a*k - j*b) + e * (j*c - a*l) + d * (b*l - k*c)) / M;
    if (!aInterval.trimRight(t))
    {
        return {};
    }

    auto beta = (j * ei_m_hf + k * gf_m_di + l * dh_m_eg) / M;
    if (beta < 0 || beta > 1)
    {
        return {};
    }

    auto gamma = (i * (a*k - j*b) + h * (j*c - a*l) + g * (b*l - k*c)) / M;
    if (gamma < 0 || (gamma + beta) > 1)
    {
        return {};
    }

    return make_hit(aRay, aTriangle, t);
}


} // namespace focg
} // namespace ad

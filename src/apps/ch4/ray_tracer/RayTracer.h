#pragma once

#include "Hit.h"
#include "Scene.h"
#include "Shading.h"
#include "View.h"

#include <arte/Image.h>

#include <optional>


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


ad::Image<> rayTrace(const Scene & aScene, const View & aView)
{
    math::Size<2, int> resolution = aView.getResolution();
    ad::Image<> image{resolution, math::sdr::gWhite};
    Interval interval;

    for (int j = 0; j != resolution.height(); ++j)
    {
        for (int i = 0; i != resolution.width(); ++i)
        {
            // The image origin is top-left, the ray tracer viewport is bottom-left
            // we take j in the image space, so it corresponds to the viewspace coordinate height-j.
            if (auto hit = intersect(aView.getRay(i, resolution.height()-j), aScene.sphere, interval))
            {
                image.at(i, j) = to_sdr(shade(hit->position, hit->normal, *(hit->material),
                                              aView.getPosition(),
                                              aScene.ambientLight, aScene.lights.begin(), aScene.lights.end()));
            }
        }
    }

    return image;
}


} // namespace focg
} // namespace ad

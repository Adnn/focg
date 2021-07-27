#include "Hit.h"
#include "Sphere.h"
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
        math::UnitVec<3>::make_fromUnitLength((aRay(t) - aSphere.center) / aSphere.radius)
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

        if (auto tmin = std::min(ta, tb); tmin < aInterval.t1)
        {
            return make_hit(aRay, aSphere, tmin);
        }
    }

    // Fall through mean no hit
    return {};

}

using Scene = Sphere;

ad::Image<> rayTrace(const Scene & aScene, const View & aView)
{
    math::Size<2, int> resolution = aView.getResolution();
    ad::Image<> image{resolution, math::sdr::gBlack};
    Interval interval;

    for (int i = 0; i != resolution.width(); ++i)
    {
        for (int j = 0; j != resolution.height(); ++j)
        {
            if (auto hit = intersect(aView.getRay(i, j), aScene, interval))
            {
                image.at(i, j) = math::sdr::gWhite;
            }
        }
    }

    return image;
}


} // namespace focg
} // namespace ad



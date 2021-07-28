#include "Surfaces.h"

#include "Intersect.h"

namespace ad {
namespace focg {


std::optional<Hit> Group::hit(const Ray & aRay, Interval aInterval) const
{
    std::optional<Hit> result;
    for (const auto& element : surfaces)
    {
        if (auto hit = element->hit(aRay, aInterval))
        {
            aInterval.trimRight(hit->t);
            result = hit;
        }
    }
    return result;
}


std::optional<Hit> Sphere::hit(const Ray & aRay, Interval aInterval) const
{
    return intersect(aRay, *this, aInterval);
}


std::optional<Hit> Triangle::hit(const Ray & aRay, Interval aInterval) const
{
    return intersect(aRay, *this, aInterval);
}

} // namespace focg
} // namespace ad

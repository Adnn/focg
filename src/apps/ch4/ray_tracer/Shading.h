#pragma once

#include "Hit.h"
#include "Ray.h"
#include "Scene.h"


namespace ad {
namespace focg {


// Forward declaration
math::hdr::Rgb getRayColor(const Ray & aRay, const Interval aInterval, const Scene & aScene,
                           int aRecursionLimit, math::hdr::Rgb aBackgroundColor);

math::hdr::Rgb shade(const Hit & aHit, const Ray & aRay, const Scene & aScene, int aRecursionLimit)
{
    const Material & material = *aHit.material;
    const math::Position<3> point = aHit.position;
    const math::UnitVec<3> normal = aHit.normal;
    math::UnitVec<3> viewDirection{-aRay.direction};

    // Ambient color
    math::hdr::Rgb color = material.ambientColor.cwMul(aScene.ambientLight);
    
    for(const auto & light : aScene.lights)
    {
        math::UnitVec<3> lightDirection{light.position - point};

        // Shadow (add current light contribution only if point is not in the light's shadow).
        if (! aScene.hit(Ray{point, lightDirection}, Interval{Interval::gEpsilon}))
        {
            math::UnitVec<3> halfDirection{lightDirection + viewDirection};
            // Diffuse and specular components
            color +=  light.intensity.cwMul(material.diffuseColor * std::max(0., normal.dot(lightDirection))
                                            + material.specularColor * std::pow(std::max(0., normal.dot(halfDirection)),
                                                                                material.phongExponent));
        }
    }

    // Mirror
    if (material.reflectionColor != math::hdr::gBlack)
    {
        // Focg 3rd p87: view direction is in the opposite direction from d (ray direction) in the book.
        math::Vec<3> reflectionDirection = 2 * (viewDirection.dot(normal)) * normal - viewDirection;
        color += material.reflectionColor.cwMul(getRayColor(Ray{point, reflectionDirection},
                                                            Interval{Interval::gEpsilon},
                                                            aScene,
                                                            aRecursionLimit,
                                                            math::hdr::gBlack));
    }

    return color;
}


math::hdr::Rgb getRayColor(const Ray & aRay, const Interval aInterval, const Scene & aScene,
                           int aRecursionLimit,
                           math::hdr::Rgb aBackgroundColor)
{
    if (auto hit = (aRecursionLimit > 0 ? aScene.hit(aRay, aInterval) : std::nullopt))
    {
        return shade(*hit, aRay, aScene, aRecursionLimit-1);
    }
    else
    {
        return aBackgroundColor;
    }
}


math::hdr::Rgb getRayColor(const Ray& aRay, const Interval aInterval, const Scene& aScene,
                           int aRecursionLimit)
{
    return getRayColor(aRay, aInterval, aScene, aRecursionLimit, aScene.backgroundColor);
}


} // namespace focg
} // namespace ad

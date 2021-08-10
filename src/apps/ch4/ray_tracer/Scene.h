#pragma once

#include "Light.h"
#include "Surfaces.h"

#include <vector>


namespace ad {
namespace focg {


struct Scene
{
    std::optional<Hit> hit(const Ray& aRay, Interval aInterval) const
    {
        return geometry->hit(aRay, aInterval); 
    }

    std::shared_ptr<Surface> geometry;
    std::vector<PointLight> lights;
    math::hdr::Rgb ambientLight{math::hdr::gWhite * 0.5};
    math::hdr::Rgb backgroundColor{math::hdr::gWhite * 0.5};
};


} // namespace focg
} // namespace ad

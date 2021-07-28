#pragma once


#include "Hit.h"
#include "Material.h"
#include "Ray.h"

#include <math/Vector.h>

#include <initializer_list>
#include <optional>
#include <memory>
#include <vector>


namespace ad {
namespace focg {


struct Surface
{
    virtual std::optional<Hit> hit(const Ray & aRay, Interval aInterval) const = 0;
};


struct Sphere : Surface
{
    std::optional<Hit> hit(const Ray & aRay, Interval aInterval) const override;

    // No aggregate initialization due to virtual function
    Sphere (std::shared_ptr<Material> aMaterial, math::Position<3> aCenter, double aRadius) :
        material(std::move(aMaterial)),
        center{std::move(aCenter)},
        radius{aRadius}
    {}

    std::shared_ptr<Material> material;
    math::Position<3> center;
    double radius;
};


struct Group : Surface
{
    std::optional<Hit> hit(const Ray & aRay, Interval aInterval) const override;

    // No aggregate initialization due to virtual function
    Group(std::initializer_list<std::shared_ptr<Surface>> aSurfaces) :
        surfaces{aSurfaces}
    {}

    std::vector<std::shared_ptr<Surface>> surfaces;
};


} // namespace focg
} // namespace ad

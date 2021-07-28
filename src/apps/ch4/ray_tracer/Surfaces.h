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


struct Group : public Surface
{
    std::optional<Hit> hit(const Ray & aRay, Interval aInterval) const override;

    // No aggregate initialization due to virtual function
    Group(std::initializer_list<std::shared_ptr<Surface>> aSurfaces) :
        surfaces{aSurfaces}
    {}

    std::vector<std::shared_ptr<Surface>> surfaces;
};


struct Sphere : public Surface
{
    std::optional<Hit> hit(const Ray & aRay, Interval aInterval) const override;

    // No aggregate initialization due to virtual function
    Sphere(std::shared_ptr<Material> aMaterial, math::Position<3> aCenter, double aRadius) :
        material(std::move(aMaterial)),
        center{std::move(aCenter)},
        radius{aRadius}
    {}

    std::shared_ptr<Material> material;
    math::Position<3> center;
    double radius;
};


struct Triangle : public Surface
{
    std::optional<Hit> hit(const Ray & aRay, Interval aInterval) const override;

    // No aggregate initialization due to virtual function
    Triangle(std::shared_ptr<Material> aMaterial, 
             math::Position<3> aPoint1, math::Position<3> aPoint2, math::Position<3> aPoint3) :
        material{std::move(aMaterial)},
        a{std::move(aPoint1)},
        b{std::move(aPoint2)},
        c{std::move(aPoint3)}
    {}

    math::UnitVec<3> getNormal() const
    {
        return math::UnitVec<3>{(b-a).cross(c-a)};
    }

    std::shared_ptr<Material> material;

    math::Position<3> a;
    math::Position<3> b;
    math::Position<3> c;
};


} // namespace focg
} // namespace ad

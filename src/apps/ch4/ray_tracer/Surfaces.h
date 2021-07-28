#pragma once

#include "Material.h"

#include <math/Vector.h>

#include <memory>


namespace ad {
namespace focg {


struct Sphere
{
    std::shared_ptr<Material> material;
    math::Position<3> center;
    double radius;
};


} // namespace focg
} // namespace ad

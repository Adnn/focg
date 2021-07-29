#pragma once

#include "Hit.h"
#include "Scene.h"
#include "Shading.h"
#include "View.h"

#include <arte/Image.h>

#include <optional>


namespace ad {
namespace focg {



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
            if (auto hit = aScene.geometry->hit(aView.getRay(i, resolution.height()-j), interval))
            {
                image.at(i, j) = to_sdr(shade(hit->position, hit->normal, *(hit->material),
                                              aView.getPosition(),
                                              aScene,
                                              aScene.ambientLight, aScene.lights.begin(), aScene.lights.end()));
            }
            else
            {
                image.at(i, j) = to_sdr(aScene.backgroundColor);
            }
        }
    }

    return image;
}


} // namespace focg
} // namespace ad

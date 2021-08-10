#pragma once

#include "Hit.h"
#include "Scene.h"
#include "Shading.h"
#include "View.h"

#include <arte/Image.h>

#include <optional>


namespace ad {
namespace focg {



ad::Image<> rayTrace(const Scene & aScene, const View & aView, const int aRecursionLimit = 5)
{
    math::Size<2, int> resolution = aView.getResolution();
    ad::Image<> image{resolution, math::sdr::gWhite};

    for (int j = 0; j != resolution.height(); ++j)
    {
        for (int i = 0; i != resolution.width(); ++i)
        {
            // The image origin is top-left, the ray tracer viewport is bottom-left
            // we take j in the image space, so it corresponds to the viewspace coordinate height-j.
            image.at(i, j) = to_sdr(getRayColor(aView.getRay(i, resolution.height()-j), Interval{}, aScene, aRecursionLimit));
        }
    }

    return image;
}


} // namespace focg
} // namespace ad

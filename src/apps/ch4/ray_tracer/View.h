#pragma once

#include "Ray.h"

#include <math/Base.h>
#include <math/Rectangle.h>


namespace ad {
namespace focg {


struct Image
{
    math::Rectangle<double> viewport;
    math::Size<2, int> resolution;

    math::Position<2> getPixelPosition(std::size_t i, std::size_t j) const
    {
        return {
            viewport.x() + viewport.width() * (i + 0.5) / resolution.width(),
            viewport.y() + viewport.height() * (j + 0.5) / resolution.height()
        };
    }
};


class View
{
public:
    View(math::Position<3> & aEyePosition,
         math::Vec<3> aViewDirection,
         math::Vec<3> aUpDirection,
         Image aImage) :
        mEyePoint{aEyePosition},
        mBase{math::OrthonormalBase<3>::MakeFromTwoVectors(-aViewDirection, aUpDirection)},
        mImage{std::move(aImage)}
    {}

    virtual Ray getRay(std::size_t i, std::size_t j) const = 0;

    math::Position<3> getPosition() const
    { return mEyePoint; }

    math::Size<2, int> getResolution() const
    { return mImage.resolution; }

protected:
    math::Position<3> mEyePoint;
    math::OrthonormalBase<3> mBase;
    Image mImage;
};


class OrthographicView : public View
{
public:
    using View::View;

    Ray getRay(std::size_t i, std::size_t j) const override;
};


inline Ray OrthographicView::getRay(std::size_t i, std::size_t j) const
{
    auto pixelPos = mImage.getPixelPosition(i, j);
    return Ray{
        mEyePoint + (mBase.u() * pixelPos.x() + mBase.v() * pixelPos.y()),
        -mBase.w()
    };
}


class PerspectiveView : public View
{
public:
    PerspectiveView(
         math::Position<3> & aEyePosition,
         math::Vec<3> aViewDirection,
         math::Vec<3> aUpDirection,
         Image aImage,
         double aImagePlaneDistance) : 
            View{aEyePosition, aViewDirection, aUpDirection, aImage},
            mImagePlaneDistance{aImagePlaneDistance}
    {}

    Ray getRay(std::size_t i, std::size_t j) const override;

private:
    double mImagePlaneDistance;
};


Ray PerspectiveView::getRay(std::size_t i, std::size_t j) const
{
    auto pixelPos = mImage.getPixelPosition(i, j);
    return Ray{
        mEyePoint,
        - mImagePlaneDistance * mBase.w() + pixelPos.x() * mBase.u() + pixelPos.y() * mBase.v()
    };
}

} // namespace focg
} // namespace ad

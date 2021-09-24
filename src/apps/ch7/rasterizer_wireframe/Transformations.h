#pragma once


#include <math/Homogeneous.h>
#include <math/Transformations.h>


namespace ad {
namespace focg {


math::AffineMatrix<4> viewportTransform(const math::Size<2, int> aImageResolution)
{
    using Number = math::real_number;
    return math::trans3d::window(
            // Origin of the box on z is **positive** 1, because of the right handed frame.
            math::Box<double>{{-1., -1., 1.}, {2., 2., 2.}},
            math::Box<double>{{-0.5, -0.5, 1.},
                              {
                                  static_cast<Number>(aImageResolution.width()),
                                  static_cast<Number>(aImageResolution.height()),
                                  2.
                              }});
}


math::AffineMatrix<4> orthographicProjection(const math::Box<double> & aOrthographicViewVolume)
{
    return math::trans3d::window(
            aOrthographicViewVolume,
            // Origin of the box on z is **positive** 1, because of the right handed frame.
            math::Box<double>{{-1., -1., 1.}, {2., 2., 2.}});
}


math::Matrix<4, 4> perspectiveProjection(const math::Box<double> & aOrthographicViewVolume)
{
    return math::trans3d::perspective(aOrthographicViewVolume.zMax(), aOrthographicViewVolume.zMin())
         * orthographicProjection(aOrthographicViewVolume);
}


math::AffineMatrix<4> cameraTransform(math::Position<3> aCameraPosition,
                                      math::Vec<3> aGazeDirection,
                                      math::Vec<3> aUpDirection = {0., 1., 0.})
{
    math::Frame<3> cameraFrame{
        aCameraPosition,
        math::OrthonormalBase<3>::MakeFromTwoVectors(-aGazeDirection, aUpDirection)
    };

    return math::trans3d::canonicalToFrame(cameraFrame);
}


template <class T_angle>
math::Matrix<4, 4> perspectiveToViewport(double aNearPlaneZ, double aFarPlaneZ,
                                         T_angle aVerticalFieldOfView,
                                         const math::Size<2, int> aImageResolution)
{
    // FoCG 3rd p157
    const double t = std::abs(aNearPlaneZ) * tan(aVerticalFieldOfView / 2.);
    const double r = t * aImageResolution.width() / aImageResolution.height();

    const math::Box<double> orthographicViewVolume{{-r, -t, aNearPlaneZ}, {2*r, 2*t, aNearPlaneZ - aFarPlaneZ}};
    return perspectiveProjection(orthographicViewVolume) * viewportTransform(aImageResolution);
}


} // namespace focg
} // namespace ad

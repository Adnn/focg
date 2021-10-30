#pragma once


#include "GraphicsPipeline.h"

#include <functional>


namespace ad {
namespace focg {


using namespace std::placeholders;  // for _1, _2, _3...


struct TransformAndLighting
{
    // TODO When the FragmentInterpolated is takend out of Vertex, it should return it by value
    // (not taking it as out param)
    HPos vertexImpl(focg::Vertex & aVertex, Vertex::FragmentInterpolated & aOut) const
    {
        aOut.color = aVertex.color;
        aOut.position_c = aVertex.pos * localToCamera;
        aOut.normal_c = aVertex.normal * localToCamera;
        aOut.uv = aVertex.uv;
        return aVertex.pos * (localToCamera * projection);
    }

    math::sdr::Rgb fragmentImpl(const Vertex::FragmentInterpolated & aIn)
    {
        constexpr math::Position<4> cameraPos_c{0., 0., 1., 1.};
        math::Vec<4> viewDirection = (cameraPos_c - aIn.position_c).normalize();
        math::Vec<4> lightDirection = (lightPosition_c - aIn.position_c).normalize();
        math::Vec<4> halfVector = (viewDirection + lightDirection).normalize();

        // Texturing
        double checkerFactor = 1.;
        constexpr int gCheckers = 40; // 10 by faces on the circumference
        if ((int)(std::floor(aIn.uv.x() * gCheckers) + std::floor(aIn.uv.y() * gCheckers)) % 2 == 0)
        {
            checkerFactor = 0.3; 
        }

        return to_sdr(
            aIn.color.cwMul(
                lightDiffuseColor * std::max(0., aIn.normal_c.dot(lightDirection))
                + lightSpecularColor * std::pow(std::max(0., aIn.normal_c.dot(halfVector)),
                                                phongExponent)
                + lightAmbiantColor
            )
            * checkerFactor);
    }

    // Uniforms
    math::AffineMatrix<4> localToCamera{math::AffineMatrix<4>::Identity()};
    math::Matrix<4, 4> projection{math::Matrix<4, 4>::Identity()};
    math::Position<4> lightPosition_c{0., 50., 100., 1.}; 
    math::hdr::Rgb lightDiffuseColor = math::hdr::gWhite  * 0.4;
    math::hdr::Rgb lightSpecularColor= math::hdr::gWhite * 0.3;
    math::hdr::Rgb lightAmbiantColor = math::hdr::gCyan * 0.2;
    double phongExponent = 15;

    std::function<HPos(focg::Vertex &, Vertex::FragmentInterpolated &)> vertex = 
        std::bind(&TransformAndLighting::vertexImpl, this, _1, _2);
    std::function<math::sdr::Rgb(const Vertex::FragmentInterpolated &)>fragment =
        std::bind(&TransformAndLighting::fragmentImpl, this, _1);
};


} // namespace focg
} // namespace ad

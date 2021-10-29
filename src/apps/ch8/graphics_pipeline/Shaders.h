#pragma once


#include "GraphicsPipeline.h"

#include <functional>


namespace ad {
namespace focg {


using namespace std::placeholders;  // for _1, _2, _3...


struct Transform
{
    HPos vertexImpl(const focg::Vertex & aVertex) const
    {
        return aVertex.pos * transformation;
    }

    static math::sdr::Rgb fragmentImpl(math::hdr::Rgb aColor)
    {
        return to_sdr(aColor);
    }

    math::Matrix<4, 4> transformation{math::AffineMatrix<4>::Identity()};

    std::function<HPos(const focg::Vertex &)> vertex = std::bind(&Transform::vertexImpl, this, _1);
    std::function<math::sdr::Rgb(math::hdr::Rgb)>fragment = &fragmentImpl;
};


struct TransformAndLighting
{
    HPos vertexImpl(focg::VertexAdvanced & aVertex) const
    {
        aVertex.normal *= localToCamera;
        aVertex.fragmentPos_c = aVertex.pos * localToCamera;
        return aVertex.pos * (localToCamera * projection);
    }

    math::sdr::Rgb fragmentImpl(math::hdr::Rgb aColor, HVec aNormal, HPos aFragmentPos_c)
    {
        constexpr math::Position<4> cameraPos_c{0., 0., 1., 1.};
        math::Vec<4> viewDirection = (cameraPos_c - aFragmentPos_c).normalize();
        math::Vec<4> lightDirection = (lightPosition_c - aFragmentPos_c).normalize();
        math::Vec<4> halfVector = (viewDirection + lightDirection).normalize();

        return to_sdr(
            aColor.cwMul(
                lightDiffuseColor * std::max(0., aNormal.dot(lightDirection))
                + lightSpecularColor * std::pow(std::max(0., aNormal.dot(halfVector)),
                                                phongExponent)
                + lightAmbiantColor
            ));
    }

    // Uniforms
    math::AffineMatrix<4> localToCamera{math::AffineMatrix<4>::Identity()};
    math::Matrix<4, 4> projection{math::Matrix<4, 4>::Identity()};
    math::Position<4> lightPosition_c{0., 50., 100., 1.}; 
    math::hdr::Rgb lightDiffuseColor = math::hdr::gWhite  * 0.4;
    math::hdr::Rgb lightSpecularColor= math::hdr::gWhite * 0.3;
    math::hdr::Rgb lightAmbiantColor = math::hdr::gCyan * 0.2;
    double phongExponent = 15;

    std::function<HPos(focg::VertexAdvanced &)> vertex = 
        std::bind(&TransformAndLighting::vertexImpl, this, _1);
    std::function<math::sdr::Rgb(math::hdr::Rgb, HVec, HPos)>fragment =
        std::bind(&TransformAndLighting::fragmentImpl, this, _1, _2, _3);
};


} // namespace focg
} // namespace ad

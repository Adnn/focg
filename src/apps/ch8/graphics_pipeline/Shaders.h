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

    static math::sdr::Rgb fragmentImpl(math::sdr::Rgb aColor)
    {
        return aColor;
    }

    math::Matrix<4, 4> transformation{math::AffineMatrix<4>::Identity()};

    std::function<HPos(const focg::Vertex &)> vertex = std::bind(&Transform::vertexImpl, this, _1);
    std::function<math::sdr::Rgb(math::sdr::Rgb)>fragment = &fragmentImpl;
};


struct TransformAndLighting
{
    HPos vertexImpl(const focg::Vertex & aVertex) const
    {
        return aVertex.pos * transformation;
    }

    static math::sdr::Rgb fragmentImpl(math::sdr::Rgb aColor)
    {
        return aColor;
    }

    math::Matrix<4, 4> transformation{math::AffineMatrix<4>::Identity()};

    std::function<HPos(const focg::Vertex &)> vertex = std::bind(&TransformAndLighting::vertexImpl, this, _1);
    std::function<math::sdr::Rgb(math::sdr::Rgb)>fragment = &fragmentImpl;
};


} // namespace focg
} // namespace ad

#pragma once


namespace ad {
namespace focg {


struct Light
{
    math::hdr::Rgb intensity;
};


struct PointLight : public Light
{
    math::Position<3> position;
};


} // namespace focg
} // namespace ad

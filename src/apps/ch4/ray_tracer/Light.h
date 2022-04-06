#pragma once


namespace ad {
namespace focg {


struct Light
{
    math::hdr::Rgb_d intensity;
};


struct PointLight : public Light
{
    math::Position<3> position;
};


} // namespace focg
} // namespace ad

#pragma once


#include "Clipping.h"
#include "Rasterization.h"
#include "Scene.h"

#include <bitset>


namespace ad {
namespace focg {


template <class T_pixel = math::sdr::Rgb, class T_depthValue = double>
struct ImageBuffer
{
    ImageBuffer(math::Size<2, int> aResolution, T_pixel aDefaultColor = T_pixel{0, 0, 0});

    math::Size<2, int> getResolution() const
    { return color.dimensions(); }

    template <class T_position>
    double & depthAt(T_position aPosition)
    { return depth[aPosition.x() + aPosition.y() * color.width()]; }

    arte::Image<T_pixel> color;
    std::vector<T_depthValue> depth;
};


template <class T_pixel, class T_depthValue>
ImageBuffer<T_pixel, T_depthValue>::ImageBuffer(math::Size<2, int> aResolution, T_pixel aDefaultColor) :
    color{aResolution, aDefaultColor},
    depth((std::size_t)aResolution.area(), std::numeric_limits<T_depthValue>::min())
{}


template <class T_targetBuffer>
struct Program
{
    using FragmentShader =
        void(*)(T_targetBuffer &, math::Position<2, int> /*aScreenPosition*/, double /*aDepth*/, math::sdr::Rgb);

    FragmentShader fragment;
};


struct GraphicsPipeline
{
private:
    using RenderFlag = std::bitset<2>;

public:
    template <class T_targetBuffer, class T_program>
    T_targetBuffer & traverse(const Scene & aScene, T_targetBuffer & aTarget, const T_program & aProgram) const;

    static constexpr RenderFlag Wireframe = 0b01;
    static constexpr RenderFlag Fill = 0b10;
    RenderFlag renderMode{Fill};
};


template <class T_targetBuffer, class T_program>
T_targetBuffer & GraphicsPipeline::traverse(const Scene & aScene,
                                            T_targetBuffer & aTarget,
                                            const T_program & aProgram) const
{
    ViewVolume volume{math::Box<double>{
        // Important 0.5 offset, because the integer coordinate are at pixel centers!
        // there is nonetheless an issue, since -0.5 rounds to -1 and 0.5 rounds to 1
        // (which tends to include adjacent pixels when clipping, thus overlapping to next line/column)
        // changing from std::round to std::nearbyint seems to handle -0.5 and 0.5 as we need, but
        // rounding errors make it not robust enough at the moment.
        //{-0.5, -0.5, 500.},
        //{static_cast<ad::math::Size<2, double>>(aResolution), 500.},

        // A Q&D solution is to restrain the view volume by some arbitrary amount
        {-0.45, -0.45, 500.},
        {static_cast<math::Size<2, double>>(aTarget.getResolution()) - math::Size<2, double>{0.1, 0.1}, 1000.},
    }};

    // TODO adress proper line drawing via shader and depth buffer
    for (const auto & line : aScene.lines)
    {
        if (auto clippedLine = clip(line, volume))
        {
            rasterizeLine(*clippedLine, aTarget.color, ad::math::sdr::gWhite);
        }
    }

    for (const auto & triangle : aScene.triangles)
    {
        for (const auto & triangle : clip(triangle, volume))
        {
            if ((renderMode & Fill).any())
            {
                rasterizeIncremental(triangle, aTarget, aProgram.fragment);
            }
            if ((renderMode & Wireframe).any())
            {
                rasterizeLine(triangle.getLineC(), aTarget.color);
                rasterizeLine(triangle.getLineB(), aTarget.color);
                rasterizeLine(triangle.getLineA(), aTarget.color);
            }
        }
    }

    return aTarget;
}

 
} // namespace focg
} // namespace ad

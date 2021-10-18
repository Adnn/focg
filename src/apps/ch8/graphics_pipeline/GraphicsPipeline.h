#pragma once


#include "Clipping.h"
#include "Rasterization.h"
#include "Scene.h"

#include <bitset>


namespace ad {
namespace focg {


/// \brief Realizee the TargetBuffer concept
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
    // Near plane > Far plane, so the test is for superiority (hence min).
    // Important: for floating point, ::min() is the lowest positive value...
    depth((std::size_t)aResolution.area(), std::numeric_limits<T_depthValue>::lowest())
{}


/// \brief Models the programmable part of the pipeline.
template <class T_vertex, class T_targetBuffer>
struct StatelessProgram
{
    using VertexShader = HPos(*)(const T_vertex & aVertex);
    using FragmentShader = math::sdr::Rgb(*)(math::sdr::Rgb);

    StatelessProgram(VertexShader aVertex, FragmentShader aFragment) :
        vertex{std::move(aVertex)},
        fragment{std::move(aFragment)}
    {}

    StatelessProgram(FragmentShader aFragment) :
        fragment{std::move(aFragment)}
    {}

    VertexShader vertex = [](const T_vertex & aVertex){return aVertex.pos;};
    FragmentShader fragment;
};


struct GraphicsPipeline
{
private:
    using RenderFlag = std::bitset<2>;

public:
    /// Have each object in aScene travers the graphics pipeline, rasterizing to aTarget.
    template <class T_targetBuffer, class T_program>
    T_targetBuffer & traverse(
        const Scene & aScene, T_targetBuffer & aTarget, const T_program & aProgram,
        double aNear, double aFar) const;

    static constexpr RenderFlag Wireframe = 0b01;
    static constexpr RenderFlag Fill = 0b10;
    RenderFlag renderMode{Fill};
};


template <class T_targetBuffer, class T_program>
T_targetBuffer & GraphicsPipeline::traverse(const Scene & aScene,
                                            T_targetBuffer & aTarget,
                                            const T_program & aProgram,
                                            double aNear, double aFar) const
{
    // The offset approach does not work well here.
    //constexpr double e = 10E-3;
    //ViewVolume volume{math::Box<double>::CenterOnOrigin({2. - e, 2. - e, 2. - e})};
    ViewVolume volume{math::Box<double>::CenterOnOrigin({2., 2., 2.})};

    // Substract {1, 1} from the resolution because we want vertices at {1.0, 1.0} in NDC
    // to end up mapped to the last pixel coordinate (which is resolution - {1, 1}).
    // This loosely corresponds the offset on the view volume in NaivePipeline.
    const math::AffineMatrix<4> viewportTransform =
        math::trans3d::ndcToViewport(
            { 
                {0., 0.},
                static_cast<math::Size<2, double>>(aTarget.getResolution() - math::Size<2, int>{1, 1}) 
            },
            aNear, aFar);

    // TODO adress proper line drawing via shader and depth buffer
    //for (const auto & line : aScene.lines)
    //{
    //    if (auto clippedLine = clip(line, volume))
    //    {
    //        rasterizeLine(*clippedLine, aTarget.color, ad::math::sdr::gWhite);
    //    }
    //}

    for (auto triangle : aScene.triangles)
    {
        // Vertex shader
        triangle.a.pos = aProgram.vertex(triangle.a);
        triangle.b.pos = aProgram.vertex(triangle.b);
        triangle.c.pos = aProgram.vertex(triangle.c);

        // Now, the vertices coordinates are expressed in clip space

        // Clipping
        for (auto & triangle : clip(triangle, volume))
        {
            // Perspective divide
            triangle.perspectiveDivide();

            // Viewport transform
            triangle.transform(viewportTransform);

            // Rasterization of primitives in viewport space
            if ((renderMode & Fill).any())
            {
                rasterizeIncremental(
                    triangle,
                    aTarget,
                    [&aProgram](T_targetBuffer & aTarget,
                        math::Position<2, int> aScreenPosition, 
                        double aFragmentDepth, 
                        math::sdr::Rgb aColor)
                    {
                        // Depth test (Z buffer)
                        // Note: Near plane > Far plane, so the test is for superiority.
                        if (aFragmentDepth > aTarget.depthAt(aScreenPosition))
                        {
                            // Fragment Shader
                            aTarget.color.at(aScreenPosition) = aProgram.fragment(aColor);
                            aTarget.depthAt(aScreenPosition) = aFragmentDepth;
                        }
                    });
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

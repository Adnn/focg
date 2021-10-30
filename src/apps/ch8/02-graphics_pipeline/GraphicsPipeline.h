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

    void clear()
    { 
        color.clear(clearColor);
        std::fill(depth.begin(), depth.end(), std::numeric_limits<T_depthValue>::lowest());
    }

    arte::Image<T_pixel> color;
    std::vector<T_depthValue> depth;
    T_pixel clearColor;
};


template <class T_pixel, class T_depthValue>
ImageBuffer<T_pixel, T_depthValue>::ImageBuffer(math::Size<2, int> aResolution, T_pixel aDefaultColor) :
    color{aResolution, aDefaultColor},
    // Near plane > Far plane, so the test is for superiority (hence min).
    // Important: for floating point, ::min() is the lowest positive value...
    depth((std::size_t)aResolution.area(), std::numeric_limits<T_depthValue>::lowest()),
    clearColor{aDefaultColor}
{}


/// \brief Models the programmable part of the pipeline.
template <class T_vertex, class T_targetBuffer>
struct StatelessProgram
{
    using VertexShader = HPos(*)(const T_vertex & aVertex);
    using FragmentShader = math::sdr::Rgb(*)(math::hdr::Rgb);

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
    template <class T_vertex, class T_targetBuffer, class T_program>
    T_targetBuffer & traverse(
        const Scene_base<T_vertex> & aScene, T_targetBuffer & aTarget, const T_program & aProgram,
        double aNear, double aFar) const;

    static constexpr RenderFlag Wireframe = 0b01;
    static constexpr RenderFlag Fill = 0b10;
    RenderFlag renderMode{Fill};
};


template <class T_vertex, class T_targetBuffer, class T_program>
T_targetBuffer & GraphicsPipeline::traverse(const Scene_base<T_vertex> & aScene,
                                            T_targetBuffer & aTarget,
                                            const T_program & aProgram,
                                            double aNear, double aFar) const
{
    // NOTE: The pipeline expects the output of the vertex processing stage to be in clip space
    // (i.e. OpenGL convention).
    // Thus, clipping is done against the simple case of unit cube.
    ViewVolume volume{math::Box<double>::CenterOnOrigin({2., 2., 2.})};

    // NOTE: The initial view volume (and the NDC unit cube) should be mapped to the whole viewport,
    // not to the pixel center range (which goes from (0, 0) to resolution - (1, 1)).
    // Since the pixel center are assigned integer indices in the viewport, its origin is at {-0.5, -0.5}.
    // NOTE: For the same reason than in NaivePipeline, the exact viewport has to be offset by a small epsilon,
    // otherwise the floating point rounding errors (and round() behaviour) might map 
    // a position exactly on the edge of the view volume to a pixel just outside the viewport.
    constexpr math::Vec<2> epsilon{0.1, 0.1};
    const math::AffineMatrix<4> viewportTransform =
        math::trans3d::ndcToViewport(
            { 
                math::Position<2>{-0.5, -0.5} + epsilon,
                static_cast<math::Size<2, double>>(aTarget.getResolution())  - 2 * epsilon.as<math::Size>()
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

    for (const auto & triangleScene : aScene.triangles)
    {
        auto triangleVertexStage = triangleScene;
        // Vertex shader
        triangleVertexStage.a.pos = aProgram.vertex(triangleVertexStage.a);
        triangleVertexStage.b.pos = aProgram.vertex(triangleVertexStage.b);
        triangleVertexStage.c.pos = aProgram.vertex(triangleVertexStage.c);

        // Now, the vertices coordinates are expressed in clip space

        // Clipping
        for (const auto & triangleClipped: clip(triangleVertexStage, volume))
        {
            auto triangle = triangleClipped;
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
                        math::hdr::Rgb aColor,
                        auto ... aExtra)
                    {
                        // Depth test (Z buffer)
                        // Note: Near plane > Far plane, so the test is for superiority.
                        if (aFragmentDepth > aTarget.depthAt(aScreenPosition))
                        {
                            // Fragment Shader
                            aTarget.color.at(aScreenPosition) = aProgram.fragment(aColor, aExtra...);
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

#pragma once


#include "GraphicsPipeline.h"
#include "ObjModels.h"
#include "Scene.h"
#include "Shaders.h"

#include <graphics/CameraUtilities.h>

#include <sstream>


namespace ad {
namespace focg {


struct Timeline
{
    bool next()
    {
        currentTime += delta;
        ++currentFrame;
        return done();
    }

    bool done() const
    {
        return currentFrame >= frameCount;
    }

    const double delta; // period in seconds
    const int frameCount;

    int currentFrame{0};
    double currentTime{0.};
};


struct AnimatedScene
{
    void update(const Timeline & aTimeline)
    {
        constexpr double rotationsPerSecond = 0.25;
        for (auto & [scene, transform] : posedScenes)
        {

            transform *= math::trans3d::rotateY(math::Radian<double>{
                rotationsPerSecond * 2 * math::pi<double> * aTimeline.delta});
        }
    }

    math::AffineMatrix<4> camera()
    {
        return graphics::getCameraTransform(cameraPosition, looksAt - cameraPosition);
    }

    math::Matrix<4, 4> projection(double aViewportRatio)
    {
        math::Box<double> projected = math::Box<double>::CenterOnOrigin({
            math::makeSizeFromHeight<double>(shownHeight, aViewportRatio), nearPlaneZ - farPlaneZ});
        projected.origin().z() = nearPlaneZ;

        return math::trans3d::perspective(nearPlaneZ, farPlaneZ)
               * math::trans3d::orthographicProjection(projected);
    }

    std::vector<std::pair<Scene_base<VertexAdvanced>, math::AffineMatrix<4>>> posedScenes;

    math::Position<3> cameraPosition{0., 0., 100.};
    math::Position<3> looksAt{0., 0., 0.};

    double shownHeight = 100;

    double nearPlaneZ = -20.;
    double farPlaneZ  = -1000.; 
};


struct ShadingRenderer
{
    ShadingRenderer(math::Size<2, int> aResolution, math::sdr::Rgb aBackgroundColor);

    void render(AnimatedScene & aAnimation,
                Timeline & aTimeline,
                const filesystem::path & aFolder,
                const std::string & aFileprefix) ;

    GraphicsPipeline pipeline;
    ImageBuffer<> renderTarget;
    TransformAndLighting program;
};


ShadingRenderer::ShadingRenderer(math::Size<2, int> aResolution, math::sdr::Rgb aBackgroundColor) :
    renderTarget{aResolution, aBackgroundColor}
{}


void ShadingRenderer::render(AnimatedScene & aAnimation,
                             Timeline & aTimeline,
                             const filesystem::path & aFolder,
                             const std::string & aFileprefix) 
{
    for(; !aTimeline.done(); aTimeline.next())
    {
        // Update animation based on time
        aAnimation.update(aTimeline);
        renderTarget.clear();

        for (const auto & [scene, localToWorld] : aAnimation.posedScenes)
        {
            program.localToCamera = localToWorld * aAnimation.camera();
            program.projection = aAnimation.projection(math::getRatio<double>(renderTarget.getResolution()));
            pipeline.traverse(scene, renderTarget, program, aAnimation.nearPlaneZ, aAnimation.farPlaneZ)
                .color.saveFile(aFolder / (aFileprefix + "-" + std::to_string(aTimeline.currentFrame) + ".ppm"),
                                arte::ImageOrientation::InvertVerticalAxis);
        }
    }
}


void renderDemoScene(const filesystem::path & aFolder,
                     math::Size<2, int> aResolution = {640, 640},
                     double aFps = 5.,
                     double aDuration = 4)
{
    AnimatedScene animation;
    {
        focg::Scene_base<VertexAdvanced> cube;
        appendToScene(std::istringstream{focg::gCubeObj}, cube, math::hdr::gCyan);

        const double cubeSize = 100.;
        math::AffineMatrix<4> modelling = 
            math::trans3d::translate(math::Vec<3>{-0.5, -0.5, -0.5})
            * math::trans3d::scale(cubeSize, cubeSize, cubeSize);

        animation.posedScenes.emplace_back(cube, modelling);
    }

    Timeline timeline{1.0 / aFps, (int)(aDuration * aFps)};

    ShadingRenderer renderer{aResolution, math::sdr::gBlack};
    //renderer.pipeline.renderMode = focg::NaivePipeline::Wireframe;
    renderer.render(animation,  timeline, aFolder, "demoscene");
}


} // namespace focg
} // namespace ad

#include "GraphicsPipeline.h"
#include "NaivePipeline.h"
#include "Scene.h"

#include <math/Color.h>
#include <math/Transformations.h>

#include <arte/Image.h>
#include <platform/Filesystem.h>

#include <cstdlib>


using namespace ad;


void lineTest(filesystem::path aImagePath, math::Size<2, int> aResolution)
{
    focg::Scene scene{
        { // vector
            // 1st quadrant
            { {400., 400.}, {600., 500.} },
            { {400., 400.}, {700., 700.} },

            // 2nd
            { {400., 400.}, {700., 710.} },
            { {400., 400.}, {410., 700.} },
            { {400., 400.}, {400., 700.} },

            // 3rd
            { {400., 400.}, {100., 700.} },

            // 4th
            { {400., 400.}, {100., 400.} },

            // 5th
            { {400., 400.}, {100., 100.} },
            { {400., 400.}, {100., 110.} },

            //// 6th
            { {400., 400.}, {100., 90.} },

            // 7th
            { {400., 400.}, {400., 100.} },
            { {400., 400.}, {500., 100.} },
            { {400., 400.}, {700., 100.} },

            // 8th
            { {400., 400.}, {700., 130.} },
            { {400., 400.}, {700., 400.} },
            
        }};

    scene.render(aResolution).saveFile(aImagePath / "ch8_lines_test.ppm",
                                       arte::ImageOrientation::InvertVerticalAxis);
}


void rgbTriangle(filesystem::path aImagePath, math::Size<2, int> aResolution)
{
    focg::Scene scene;
    scene.triangles = {
        {/*triangle*/
            { {300., 300., 0., 1.}, math::hdr::gRed },
            { {500., 300., 0., 1.}, math::hdr::gGreen },
            { {400., 500., 0., 1.}, math::hdr::gBlue },
        }
    };

    scene.render(aResolution).saveFile(aImagePath / "ch8_triangles_test.ppm",
                                       arte::ImageOrientation::InvertVerticalAxis);
}


focg::Scene lineClipping()
{
    return focg::Scene{
        { // vector
            // clip left
            { {-400., 400.}, {600., 500.} },
            // clip right
            { {400., 400.}, {11700., 700.} },
            // clip top and bottom
            { {400., -400.}, {700., 5000.} },

            // no clip
            { {200., 200.}, {600., 100.} },
            // clip front and back
            { {200., 180., 1000., 1.}, {600., 80., -1000., 1.} },
        }};
}


focg::Scene triangleClipping()
{
    focg::Scene scene;
    scene.triangles = {
        // all windows borders
        {/*triangle*/
            { {-100., -200., 0., 1.}, math::hdr::gBlack },
            { { 900.,  200., 0., 1.}, math::hdr::gYellow },
            { { 400., 1000., 0., 1.}, math::hdr::gCyan },
        },

        // Left + top
        {
            { {-100.,  400., 0., 1.}, math::hdr::gBlack },
            { {   0., 1000., 0., 1.}, math::hdr::gYellow },
            { { 300.,  500., 0., 1.}, math::hdr::gCyan },
        },

        // left edge
        {/*triangle*/
            { {-100., 300., 0., 1.}, math::hdr::gRed },
            { { 500., 300., 0., 1.}, math::hdr::gGreen },
            { { 400., 500., 0., 1.}, math::hdr::gBlue },
        },
        // top edge
        {/*triangle*/
            { {100.,  600., 0., 1.}, math::hdr::gRed },
            { {500.,  500., 0., 1.}, math::hdr::gGreen },
            { {400., 1500., 0., 1.}, math::hdr::gBlue },
        },
        // right edge
        {/*triangle*/
            { { 550., 300., 0., 1.}, math::hdr::gRed },
            { { 900., 300., 0., 1.}, math::hdr::gGreen },
            { { 450., 500., 0., 1.}, math::hdr::gBlue },
        },
        // bottom edge
        {/*triangle*/
            { {400.,  100., 0., 1.}, math::hdr::gRed },
            { {600., -100., 0., 1.}, math::hdr::gGreen },
            { {700.,  100., 0., 1.}, math::hdr::gBlue },
        },
        {/*triangle*/
            { {200.,  100., 0., 1.}, math::hdr::gRed },
            { {100., -100., 0., 1.}, math::hdr::gGreen },
            { {400., -100., 0., 1.}, math::hdr::gBlue },
        },
    };
    return scene;
}


focg::Scene depthBuffer()
{
    focg::Scene scene;
    scene.triangles = {
        {
            { {100.,  100., -10., 1.}, math::hdr::gRed  },
            { {600.,  400.,  10., 1.}, math::hdr::gBlue  },
            { {100.,  700., -10., 1.}, math::hdr::gGreen  },
        },
        {
            { {700.,  100., -10., 1.}, math::hdr::gCyan  },
            { {200.,  400.,  10., 1.}, math::hdr::gYellow  },
            { {700.,  700., -10., 1.}, math::hdr::gMagenta  },
        }
    };

    return scene;
}

void renderImage(const focg::Scene & aScene,
                 const focg::NaivePipeline & aPipeline,
                 filesystem::path aImageFilePath,
                 math::Size<2, int> aResolution)
{
    aPipeline.traverse(aScene, aResolution)
        .saveFile(aImageFilePath, arte::ImageOrientation::InvertVerticalAxis);
}

void renderImage(const focg::Scene & aScene,
                 const focg::GraphicsPipeline & aPipeline,
                 filesystem::path aImageFilePath,
                 math::Size<2, int> aResolution)
{
    using Buffer = focg::ImageBuffer<>;
    Buffer targetBuffer{aResolution};

    focg::Program<focg::Vertex, Buffer> program{
        [](const focg::Vertex & aVertex) -> focg::HPos
        {
            static math::AffineMatrix<4> transform = 
                math::trans3d::translate<double>({-400., -400., 0.})
                * math::trans3d::rotateZ(math::Degree<double>{45.})
                * math::trans3d::translate<double>({ 400., 400., 0.})
                ;
            return aVertex.pos * transform;
        },
        [](math::sdr::Rgb aColor)
        {
            return aColor;
        }
    };

    aPipeline.traverse(aScene, targetBuffer, program)
        .color.saveFile(aImageFilePath, arte::ImageOrientation::InvertVerticalAxis);
}


void renderAll(filesystem::path aImagePath, math::Size<2, int> aResolution)
{

    lineTest(aImagePath, aResolution);
    rgbTriangle(aImagePath, aResolution);

    focg::NaivePipeline pipeline;
    pipeline.renderMode = focg::NaivePipeline::Wireframe | focg::NaivePipeline::Fill;
    //pipeline.renderMode = focg::NaivePipeline::Wireframe;
    renderImage(lineClipping(),     pipeline, aImagePath / "ch8_clipping_lines.ppm",     aResolution);
    renderImage(triangleClipping(), pipeline, aImagePath / "ch8_clipping_triangles.ppm", aResolution);

    focg::GraphicsPipeline pipelineZBuffered;
    //pipelineZBuffered.renderMode = focg::NaivePipeline::Wireframe | focg::NaivePipeline::Fill;
    renderImage(depthBuffer(), pipelineZBuffered, aImagePath / "ch8_depth_buffer.ppm", aResolution);
}


int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " output_image_folder\n";
        return EXIT_FAILURE;
    }

    try 
    {   
        renderAll(argv[1], {800, 800});
        return EXIT_SUCCESS;
    }
    catch (std::exception & e)
    {
        std::cerr << "Uncaught exception: " << e.what();
        return EXIT_FAILURE;
    }
}

#include "GraphicsPipeline.h"
#include "NaivePipeline.h"
#include "ObjLoader.h"
#include "ObjModels.h"
#include "Scene.h"
#include "Shaders.h"

#include <arte/Image.h>

#include <graphics/CameraUtilities.h>

#include <math/Color.h>
#include <math/Transformations.h>

#include <platform/Filesystem.h>

#include <sstream>

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
            { {-0.8, -0.8, -0.5, 1.}, math::hdr::gRed  },
            { { 0.2,  0.0,  0.5, 1.}, math::hdr::gBlue  },
            { {-0.8,  0.8, -0.5, 1.}, math::hdr::gGreen  },
        },
        {
            { { 0.8, -0.8, -0.5, 1.}, math::hdr::gCyan  },
            { {-0.2,  0.0,  0.5, 1.}, math::hdr::gYellow  },
            { { 0.8,  0.8, -0.5, 1.}, math::hdr::gMagenta  },
        }
    };

    return scene;
}


void renderPerspectiveCube(filesystem::path aImageFilePath, math::Size<2, int> aResolution)
{
    focg::Scene scene;
    appendToScene(std::istringstream{focg::gCubeObj}, scene);

    focg::GraphicsPipeline pipeline;
    pipeline.renderMode = focg::NaivePipeline::Wireframe;

    using Buffer = focg::ImageBuffer<>;
    Buffer targetBuffer{aResolution};

    focg::TransformAndLighting program;

    const double nearPlaneZ = -60;
    const double farPlaneZ = -150;
    const double depth = nearPlaneZ - farPlaneZ;

    const double cubeSize = 100.;
    math::AffineMatrix<4> modelling = 
        math::trans3d::translate(math::Vec<3>{-0.5, -0.5, -0.5})
        * math::trans3d::scale(cubeSize, cubeSize, cubeSize);

    math::Position<3> cameraPosition{0., 0., 100.};
    math::Position<3> looksAt{0., 0., 0.};
    math::Vec<3> gazeDirection = looksAt - cameraPosition;
    //math::Vec<3> gazeDirection{0., 0., -1.};
    math::AffineMatrix<4> camera = graphics::getCameraTransform(cameraPosition, gazeDirection);

    // NOTE: Negate the perspective matrix, this way -Z is copied into W, which fixes
    // the clipping tests (and respects OpenGL convetion that the test is -w < x, y, z < +w)
    //math::Matrix<4, 4> perspective = math::trans3d::perspective(nearPlaneZ, farPlaneZ);
    math::Matrix<4, 4> perspective = math::trans3d::perspective(nearPlaneZ, farPlaneZ) * -1;

    const double shownHeight = 100;
    math::Box<double> projected = math::Box<double>::CenterOnOrigin({
        math::makeSizeFromHeight<double>(shownHeight, math::getRatio<double>(aResolution)),
        depth});
    projected.origin().z() = nearPlaneZ;
    std::cout << "Projected box in world: " << projected << "\n";

    math::AffineMatrix<4> orthographic = math::trans3d::orthographicProjection(projected);
    math::Matrix<4, 4> customProjection = perspective * orthographic; 

    math::Matrix<4, 4> openGLPerspectiveProjection = [&](){
        double n = std::abs(nearPlaneZ);
        double f = std::abs(farPlaneZ);
        double l = -shownHeight/2.;
        double r = shownHeight/2.;
        double b = -shownHeight / 2.;
        double t = shownHeight / 2.;

        return math::Matrix<4, 4>{
            2 * n / (r-l),  0.,             0.,             0.,
            0.,             2*n / (t - b),  0.,             0.,
            (r+l)/(r-l),    (t+b)/(t-b),    (n+f)/(n-f),    -1.,
            0.,             0.,             2*f*n/(n-f),    0.
        };
    }();

    std::cout << "\nPerspective:\n" << (perspective).transpose()
              << "\n\nOrthographic:\n" << (orthographic).transpose()
              << "\n\nCustom Complete projection:\n" << (customProjection).transpose()
              << "\n\nOpenGL Complete projection:\n" << (openGLPerspectiveProjection).transpose() 
              << "\n";


    program.transformation = 
        modelling 
        * camera
        * customProjection
        //* openGLPerspectiveProjection
        ;

    pipeline.traverse(scene, targetBuffer, program, nearPlaneZ, farPlaneZ)
        .color.saveFile(aImageFilePath, arte::ImageOrientation::InvertVerticalAxis);
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

    focg::StatelessProgram<focg::Vertex, Buffer> program{
        [](const focg::Vertex & aVertex) -> focg::HPos
        {
            static math::AffineMatrix<4> transform = 
                math::trans3d::rotateZ(math::Degree<double>{45.});
            return aVertex.pos * transform;
        },
        [](math::sdr::Rgb aColor)
        {
            return aColor;
        }
    };

    // Hardcoded depth planes
    aPipeline.traverse(aScene, targetBuffer, program, 500, -500)
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

    renderPerspectiveCube(aImagePath / "ch8_perspective_cube.ppm", aResolution);
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

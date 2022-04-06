#include "DrawLine.h"
#include "Rasterize.h"
#include "Scene.h"
#include "Transformations.h"

#include <math/Color.h>

#include <arte/Image.h>
#include <platform/Filesystem.h>

#include <cstdlib>


using namespace ad;


void render(filesystem::path aImagePath, math::Size<2, int> aResolution)
{
    // Defines a cube from [100, 100, 100] to [200, 200, 200]
    focg::Scene scene{
        { // vector
            // front face
            { {100., 100., 100.}, {200., 100., 100.} },
            { {200., 100., 100.}, {200., 200., 100.} },
            { {200., 200., 100.}, {100., 200., 100.} },
            { {100., 200., 100.}, {100., 100., 100.} },
            // back face
            { {100., 100., 200.}, {200., 100., 200.} },
            { {200., 100., 200.}, {200., 200., 200.} },
            { {200., 200., 200.}, {100., 200., 200.} },
            { {100., 200., 200.}, {100., 100., 200.} },
            // front-to-back lines
            { {100., 100., 100.}, {100., 100., 200.} },
            { {200., 100., 100.}, {200., 100., 200.} },
            { {200., 200., 100.}, {200., 200., 200.} },
            { {100., 200., 100.}, {100., 200., 200.} },
        }};



    math::AffineMatrix<4> viewingTransform =
        focg::orthographicProjection(math::Box<double>{ {0., 0., 0.}, {300., 300., 300.} })
        * focg::viewportTransform(aResolution)
        ;
    rasterize(scene, viewingTransform, aResolution).saveFile(aImagePath / "ch7_rasterized_ortho.ppm");

    math::Position<3> cameraPosition{100., 250., 300.};
    math::Vec<3> gazeDirection = math::Position<3>{150., 150., 150.} - cameraPosition;

    viewingTransform =
          focg::cameraTransform(cameraPosition, gazeDirection)
        * focg::orthographicProjection(math::Box<double>{ {-150., -150., -300.}, {300., 300., 300.} })
        * focg::viewportTransform(aResolution)
        ;
    rasterize(scene, viewingTransform, aResolution).saveFile(aImagePath / "ch7_rasterized_ortho_moved.ppm");

    math::Matrix<4, 4> perspectiveViewingTransform =
          focg::cameraTransform(cameraPosition, gazeDirection)
        * focg::perspectiveProjection(math::Box<double>{ {-150., -150., -500.}, {300., 300., 300.} })
        * focg::viewportTransform(aResolution)
        ;
    rasterize(scene, perspectiveViewingTransform, aResolution).saveFile(aImagePath / "ch7_rasterized_perpsective.ppm");

    perspectiveViewingTransform =
          focg::cameraTransform(cameraPosition, gazeDirection)
        * focg::perspectiveToViewport(-200., -500., math::Degree<double>{75.}, aResolution)
        ;
    rasterize(scene, perspectiveViewingTransform, aResolution).saveFile(aImagePath / "ch7_rasterized_perpsective_fov.ppm");
}

int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " output_image_folder\n";
        return EXIT_FAILURE;
    }

    render(argv[1], {800, 800});

    return EXIT_SUCCESS;
}

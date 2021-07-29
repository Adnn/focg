#include "RayTracer.h"
#include "Surfaces.h"
#include "View.h"

#include <math/Color.h>

#include <cstdlib>


using namespace ad;


void render(filesystem::path aImagePath, math::Size<2, int> aResolution)
{
    focg::Image viewport{
        math::Rectangle<double>{
            {-150., -150.},
            {300., 300.}
        },
        aResolution
    };

    focg::OrthographicView orthographic{
        math::Position<3>{0., 100., 0.},
        {0., -100., -100.},
        {0., 1., 0.},
        viewport
    };

    math::Position<3> perspectivePosition{-0., 600., 1000.};
    focg::PerspectiveView perspective{
        perspectivePosition, 
        math::Position<3>{0., 0., -100.} - perspectivePosition,
        {0., 1., 0.},
        viewport,
        800 
    };

    math::hdr::Rgb sphereSpecularColor{math::hdr::gWhite * 0.5};
    double colorIntensity = 0.7;
    auto cyanMaterial = std::make_shared<focg::Material>(
        focg::Material{math::hdr::gCyan*colorIntensity, math::hdr::gCyan*colorIntensity, sphereSpecularColor, 75});

    auto magentaMaterial = std::make_shared<focg::Material>(*cyanMaterial);
    magentaMaterial->ambientColor = magentaMaterial->diffuseColor = math::hdr::gMagenta*colorIntensity;
    magentaMaterial->specularColor = math::hdr::gBlack;

    auto blueMaterial = std::make_shared<focg::Material>(*cyanMaterial);
    blueMaterial->ambientColor = blueMaterial->diffuseColor = math::hdr::Rgb{77./255, 100./255, 141./255};
    blueMaterial->specularColor = math::hdr::gWhite*0.8;
    blueMaterial->phongExponent = 100;

    auto root = std::make_shared<focg::Group>(focg::Group{
            std::make_shared<focg::Sphere>(
                cyanMaterial,
                math::Position<3>{-55., 0., -80.},
                50.),
            std::make_shared<focg::Sphere>(
                magentaMaterial,
                math::Position<3>{55., 0., -110.},
                50.),
            std::make_shared<focg::Triangle>(
                blueMaterial,
                math::Position<3>{-360., -50., 0.},
                math::Position<3>{360., -50., 0.},
                math::Position<3>{0., -50., -360.}),
    });

    math::hdr::Rgb lightIntensity{math::hdr::gWhite * 0.5};
    focg::Scene scene{
        std::move(root),
        std::vector<focg::PointLight>{
            //{math::hdr::Rgb{0., 0., 0.9}, math::Position<3>{200., 100., 0.}},
            //{math::hdr::Rgb{0.9, 0., 0.}, math::Position<3>{-200., 100., 0.}},
            //{math::hdr::Rgb{0., 0.9, 0.}, math::Position<3>{0., -240., 0.}},

            //{lightIntensity, math::Position<3>{200., 100., 0.}},
            //{lightIntensity, math::Position<3>{-200., 100., 0.}},
            //{lightIntensity, math::Position<3>{0., -240., 0.}},

            {math::hdr::gWhite * 0.7, math::Position<3>{-3000., 2000., -50.}},
            {math::hdr::gWhite * 0.3, math::Position<3>{0., 0., 1000.}},
            {math::hdr::gWhite * 0.45, math::Position<3>{3000., 10000., 0.}},

        },
        math::hdr::Rgb{math::hdr::gWhite * 0.5}
    };

    //rayTrace(scene, orthographic).saveFile(aImagePath);
    rayTrace(scene, perspective).saveFile(aImagePath);
}

int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " output_image_path\n";
        return EXIT_FAILURE;
    }

    render(argv[1], {800, 800});

    return EXIT_SUCCESS;
}

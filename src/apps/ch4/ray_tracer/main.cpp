#include "RayTracer.h"
#include "View.h"

#include <math/Color.h>

#include <cstdlib>


using namespace ad;


void render(filesystem::path aImagePath, math::Size<2, int> aResolution)
{
    focg::OrthographicView orthographic{
        math::Position<3>{0., 0., 0.},
        {0., 0., -1.},
        {0., 1., 0.},
        focg::Image{
            math::Rectangle<double>{
                {-100., -100.},
                {200., 200.}
            },
            aResolution
        }
    };

    math::hdr::Rgb sphereColor{0.2, 0.2, 0.6};
    math::hdr::Rgb sphereSpecularColor{0.3, 0.3, 0.6};
    auto material = std::make_shared<focg::Material>(focg::Material{sphereColor, sphereColor, sphereSpecularColor, 25});
    focg::Scene scene{
        focg::Sphere{
            material,
            math::Position<3>{0., 0., -100.},
            50.,
        },
        std::vector<focg::PointLight>{
            {math::hdr::Rgb{0.8, 0.8, 0.8}, math::Position<3>{60., 30., 0.}},
        },
        math::hdr::Rgb{math::hdr::gWhite * 0.7}
    };

    rayTrace(scene, orthographic).saveFile(aImagePath);
}

int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " output_image_path\n";
        return EXIT_FAILURE;
    }

    render(argv[1], {640, 640});

    return EXIT_SUCCESS;
}

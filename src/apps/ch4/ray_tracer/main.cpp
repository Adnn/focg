#include "RayTracer.h"
#include "View.h"

#include <cstdlib>


using namespace ad;


void trace(filesystem::path aImagePath, math::Size<2, int> aResolution)
{
    focg::OrthographicView orthographic{
        math::Position<3>{0., 0., 0.},
        {0., 0., 1.},
        {0., 1., 0.},
        focg::Image{
            math::Rectangle<double>{
                {-100., -100.},
                {200., 200.}
            },
            aResolution
        }
    };

    focg::Scene scene = focg::Sphere{
        {0., 0., -50.},
        50.
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

    trace(argv[1], {640, 640});

    return EXIT_SUCCESS;
}

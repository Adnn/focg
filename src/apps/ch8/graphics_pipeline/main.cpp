#include "Scene.h"

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

            // 6th
            { {400., 400.}, {100., 90.} },

            // 7th
            { {400., 400.}, {400., 100.} },
            { {400., 400.}, {500., 100.} },
            { {400., 400.}, {700., 100.} },

            // 8th
            { {400., 400.}, {700., 130.} },
            { {400., 400.}, {700., 400.} },
            
        }};

    scene.render(aResolution).saveFile(aImagePath / "ch8_lines_test.ppm");
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
        render(argv[1], {800, 800});
        return EXIT_SUCCESS;
    }
    catch (std::exception & e)
    {
        std::cerr << "Uncaught exception: " << e.what();
        return EXIT_FAILURE;
    }
}

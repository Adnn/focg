#include "BayerPattern.h"

#include <arte/Image.h>

#include <platform/Filesystem.h>

#include <cstdlib>


using namespace ad;


void exercise_1(filesystem::path aImagePath)
{
    auto colorImage = Image<>::LoadFile(aImagePath);
    filesystem::path folder = aImagePath.parent_path();
    filesystem::path stem = aImagePath.stem();

    BayerPattern pattern{{
        {{ // LinePattern::vector
            BayerPattern::Sample {
                2,
                {{ // std::array member C array
                    {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}},
                    {{-1,  0}, { 0, 1}, {1, 0}, {0, -1}},
                    {{ 0,  0}},
                }}
            },
            {
                1,
                {{
                    {{ 0, -1}, { 0,  1}},
                    {{ 0,  0}},
                    {{-1,  0}, { 1,  0}},
                }}
            },

        }},
        {{
            {
                1,
                {{
                    {{-1,  0}, { 1,  0}},
                    {{ 0,  0}},
                    {{ 0, -1}, { 0,  1}},
                }}
            },
            {
                0,
                {{
                    {{ 0,  0}},
                    {{-1,  0}, { 0, 1}, {1, 0}, {0, -1}},
                    {{-1, -1}, {-1, 1}, {1, 1}, {1, -1}},
                }}
            },

        }},
    }};

    // 1. Filtering
    Image<math::sdr::Grayscale> filtered = pattern.filter(colorImage);
    filtered.saveFile(folder / (stem.string() + "_bayerfiltered.pgm") );

    // 2. Color-coded filtered image
    pattern.colorCode(filtered).saveFile(folder / (stem.string() + "_bayerfiltered_colorcoded.ppm") );

    // 3. Demosaicing
    pattern.demosaic(filtered).saveFile(folder / (stem.string() + "_bayerfiltered_demosaic.ppm") );
}


int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " image_path\n";
        return EXIT_FAILURE;
    }

    exercise_1(argv[1]);

    return EXIT_SUCCESS;
}

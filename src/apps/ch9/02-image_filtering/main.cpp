#include "../01-convolution_tests/Convolution.h"
#include "../01-convolution_tests/Filters.h"

#include <arte/Image.h>

#include <cstdlib>


using namespace ad;


void filter(std::filesystem::path aImagePath)
{
    auto image = arte::Image<math::sdr::Rgb>::LoadFile(aImagePath);

    std::filesystem::path folder = aImagePath.parent_path();
    std::filesystem::path stem = aImagePath.stem();

    {
        std::array<double, 1> impulse{1};
        arte::Image<math::sdr::Rgb> filtered = focg::filterSeparable2D(impulse, image);

        // Save in working dir
        filtered.saveFile((stem.string() + "_identity.ppm"));
    }

    {
        constexpr int gScale = 2;
        // unscaled trimmed Gaussian, with standard deviation s*sigma and radius s*r
        auto unscaledGaussian = focg::discreteGaussian<gScale * 3>(1.0 * gScale);
        arte::Image<math::sdr::Rgb> filtered = focg::filterSeparable2D(unscaledGaussian, image);

        filtered.saveFile((stem.string() + "_gaussian_unscaled.ppm"));

        // trimmed Gaussian, scaled by s
        // Note: radius of support is s*r because:
        // "A filter that has a natural radius of r and is used at scale s has a radius of support sr
        auto scaledGaussian = focg::discreteGaussian<gScale * 3>(1.0, (double)gScale);
        filtered = focg::filterSeparable2D(scaledGaussian, image);

        filtered.saveFile((stem.string() + "_gaussian_scaled.ppm"));
    }

    // Introduces a lot of artifacts, I suppose because of unsigned arithmetic
    {
        const double alpha = 1.2;
        auto sharpen = focg::discreteSharpen<3>(alpha);
        arte::Image<math::sdr::Rgb> filtered = focg::filterSeparable2D(sharpen, image);

        filtered.saveFile((stem.string() + "_sharpen_hasartifacts.ppm"));
    }
}

int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " image_path\n";
        return EXIT_FAILURE;
    }

    filter(argv[1]);

    return EXIT_SUCCESS;
}

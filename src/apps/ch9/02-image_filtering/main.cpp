#include "../01-convolution_tests/Convolution.h"
#include "../01-convolution_tests/Filters.h"
#include "../01-convolution_tests/Reconstruction.h"

#include <arte/Image.h>

#include <cstdlib>


using namespace ad;


void filter(std::filesystem::path aImagePath)
{
    using SdrImage = arte::Image<math::sdr::Rgb>;
    using HdrImage = arte::Image<math::hdr::Rgb_f>;

    HdrImage image = to_hdr(SdrImage::LoadFile(aImagePath));

    std::filesystem::path folder = aImagePath.parent_path();
    std::filesystem::path stem = aImagePath.stem();

    {
        std::array<double, 1> impulse{1};
        auto filtered = focg::filterSeparable2D(impulse, image);

        // Save in working dir
        tonemap(filtered).saveFile((stem.string() + "_identity.ppm"));
    }

    {
        constexpr int gScale = 2;
        // unscaled trimmed Gaussian, with standard deviation s*sigma and radius s*r
        auto unscaledGaussian = focg::discreteGaussian<gScale * 3>(1.0 * gScale);
        auto filtered = focg::filterSeparable2D(unscaledGaussian, image);

        tonemap(filtered).saveFile((stem.string() + "_gaussian_unscaled.ppm"));

        // trimmed Gaussian, scaled by s
        // Note: radius of support is s*r because:
        // "A filter that has a natural radius of r and is used at scale s has a radius of support sr
        auto scaledGaussian = focg::discreteGaussian<gScale * 3>(1.0, (double)gScale);
        filtered = focg::filterSeparable2D(scaledGaussian, image);

        tonemap(filtered).saveFile((stem.string() + "_gaussian_scaled.ppm"));
    }

    {
        const double alpha = 2.1;
        auto sharpen = focg::discreteSharpen<3>(alpha);
        auto filtered = focg::filterSeparable2D(sharpen, image);

        tonemap(filtered).saveFile((stem.string() + "_sharpen.ppm"));
    }

    {
        auto resampled = HdrImage::makeUninitialized(image.dimensions() / 3);
        {
            focg::Filter filter{
                [](double x){ return focg::gaussian(x); },
                3.0,
            };

            focg::resampleSeparable2D(image, resampled, filter);

            tonemap(resampled).saveFile((stem.string() + "_downscale_filter.ppm"));
        }

        {
            focg::pointResample2D(image, resampled);
            tonemap(resampled).saveFile((stem.string() + "_downscale_closest.ppm"));
        }
    }


    {
        auto resampled = HdrImage::makeUninitialized(image.dimensions() * 3);
        {
            // Introduces a darker grid artifact
            //focg::Filter filter{
            //    [](double x){ return focg::gaussian(x, 1., 1./3); },
            //    3. * 1./3,
            //};
            focg::Filter filter{
                [](double x){ return focg::gaussian(x, 1., 2./3); },
                3. * 2./3,
            };

            focg::resampleSeparable2D(image, resampled, filter);

            tonemap(resampled).saveFile((stem.string() + "_upscale_filter.ppm"));
        }

        {
            focg::pointResample2D(image, resampled);
            tonemap(resampled).saveFile((stem.string() + "_upscale_closest.ppm"));
        }
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

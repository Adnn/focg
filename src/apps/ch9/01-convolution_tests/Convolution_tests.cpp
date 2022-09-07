#include "Convolution.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <vector>


using namespace ad;

using namespace Catch::literals;
using Catch::Approx;


SCENARIO("1D symmetric convolution")
{
    GIVEN("A filter and a sequence")
    {
        std::array<double, 3> filter{0.25, 0.5, 0.25};
        std::array<double, 6> sequence{0, 1, 2, 3, 4, 5};
        WHEN("They are convolved.")
        {
            std::vector<double> results;

            for (std::size_t i = 0; i != std::size(sequence); ++i)
            {
                results.push_back(focg::convolveDiscrete1D(filter, sequence, i));
            }

            THEN("The result is as expected.")
            {
                std::vector<double> expected {0.25, 1, 2, 3, 4, 3.5};
                CHECK(results == expected);
            }
        }
    }
}


SCENARIO("1D asymmetric convolution")
{
    GIVEN("A filter and a sequence")
    {
        std::array<double, 3> filter{0.5, 0.5, 2.};
        std::array<double, 6> sequence{0, 1, 2, 3, 4, 5};
        WHEN("They are convolved.")
        {
            std::vector<double> results;

            for (std::size_t i = 0; i != std::size(sequence); ++i)
            {
                results.push_back(focg::convolveDiscrete1D(filter, sequence, i));
            }

            THEN("The result is as expected.")
            {
                std::vector<double> expected {0.5, 1.5, 4.5, 7.5, 10.5, 10.5};
                CHECK(results == expected);
            }
        }
    }
}


SCENARIO("Box and Step")
{
    GIVEN("A filter and a sequence")
    {
        std::vector<double> filter(5, 1.0/5);
        std::vector<double> sequence(10, 0.f);
        for (std::size_t i = 0;  i != 10; ++i)
        {
            sequence.push_back(1.0);
        }

        WHEN("They are convolved.")
        {
            std::vector<double> results = focg::convolveSequence1D(filter, sequence);

            THEN("The result is as expected.")
            {
                std::vector<double> expected {0.5, 1.5, 4.5, 7.5, 10.5, 10.5};
                CHECK(results[5]  == 0);
                CHECK(results[9]  == 0.4_a);
                CHECK(results[10] == 0.6_a);
                CHECK(results[11] == 0.8_a);
                CHECK(results[15] == 1);
            }
        }
    }
}


SCENARIO("Discrete impulse")
{
    GIVEN("A sequence")
    {
        std::vector<double> sequence{0, 1, 2, 3, 4, 5};

        WHEN("It is convolved with the discrete impulse")
        {
            std::vector<double> results =
                focg::convolveSequence1D(focg::gDiscreteImpule<double>, sequence);

            THEN("The result is as the initial sequence.")
            {
                CHECK(results == sequence);
            }
        }
    }
}


SCENARIO("2D symmetric convolution")
{
    GIVEN("A filter and a sequence")
    {
        math::Matrix<3, 3, double> filter{
            1., 1., 1.,
            1., 8., 1.,
            1., 1., 1.,
        };
        filter /= 16.;

        math::Matrix<4, 4, double> sequence{
             0.,  1.,  2.,  3.,
            10., 11., 12., 13.,
            20., 21., 22., 23.,
            30., 31., 32., 33.,
        };

        WHEN("They are convolved.")
        {
            math::Matrix<4, 4, double> result = focg::convolveSequence2D(filter, sequence);

            THEN("We get the expected results.")
            {
                // Interesting property of this matrix, its convolution gives the element
                CHECK(focg::convolveDiscrete2D(filter, sequence, {1u, 1u}) == sequence[1][1]);
                CHECK(focg::convolveDiscrete2D(filter, sequence, {2u, 2u}) == sequence[2][2]);

                for (int i = 1; i != 3; ++i)
                {
                    for (int j = 1; j != 3; ++j)
                    {
                        CHECK(result[i][j] == sequence[i][j]);
                    }
                }
            }
        }
    }
}


SCENARIO("2D sepearable filtering")
{
    GIVEN("A 1D filter and a 2D sequence")
    {
        std::array<double, 3> filter1D{ 0.2, 0.6, 0.2 };

        math::Matrix<4, 4, double> sequence{
             0.,  1.,  2.,  3.,
            10., 11., 12., 13.,
            20., 21., 22., 23.,
            30., 31., 32., 33.,
        };

        GIVEN("The equivalent 2D filter")
        {
            auto filter2D = math::Matrix<3, 3, double>::Zero();
            for (std::size_t i = 0; i != std::size(filter1D); ++i)
            {
                for (std::size_t j = 0; j != std::size(filter1D); ++j)
                {
                    filter2D[i][j] = filter1D[i] * filter1D[j];
                }
            }

            WHEN("The separable filter is convolved with the whole sequence.")
            {
                math::Matrix<4, 4, double> result = focg::filterSeparable2D(filter1D, sequence);

                THEN("We get the same results as convolving with the 2D filter.")
                {
                    // Border behaviour is not the same between the two, one does "zero" the other does "border".
                    for (std::size_t i = 1; i != 3; ++i)
                    {
                        for (std::size_t j = 1; j != 3; ++j)
                        {
                            CHECK(result[i][j] == Approx(focg::convolveDiscrete2D(filter2D, sequence, {i, j})));
                        }
                    }
                }
            }
        }

    }
}

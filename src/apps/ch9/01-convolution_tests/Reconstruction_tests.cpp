#include "Reconstruction.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>
#include <vector>


using namespace ad;

using namespace Catch::literals;


SCENARIO("1D box reconstruction")
{
    GIVEN("A continuous box filter of radius 2.")
    {
        auto f = [](double x)
        {
            if (x >= -2 || x < 2)
            {
                return 1.0/4;
            }
            else
            {
                return 0.0;
            }
        };

        focg::Filter box{
            f,
            2.0,
        };

        GIVEN("a discrete step sequence")
        {
            std::vector<double> step(10, 0.f);
            for (std::size_t i = 0;  i != 10; ++i)
            {
                step.push_back(1.0);
            }

            WHEN("The sequence is reconstructed at discrete values")
            {
                THEN("It matches expected values")
                {
                    // 10 is the middle point of the sequence (i.e. the first value 1)
                    // Note: I am not sure if this first one should be 0.75, or 0.5 excluding the last.
                    CHECK(focg::reconstruct1D(step, box, 10.0) == 0.75_a);

                    CHECK(focg::reconstruct1D(step, box, 10.5) == 0.75_a);
                    CHECK(focg::reconstruct1D(step, box, 10.99) == 0.75_a);

                    CHECK(focg::reconstruct1D(step, box, 11.1) == 1.0_a);
                    CHECK(focg::reconstruct1D(step, box, 11.99) == 1.0_a);

                    CHECK(focg::reconstruct1D(step, box, 15.61) == 1.0_a);
                }
            }

            WHEN("The sequence is reconstructed at edges")
            {
                THEN("It clamps")
                {
                    CHECK(focg::reconstruct1D(step, box, 0.0) == 0.0_a);
                    CHECK(focg::reconstruct1D(step, box, 19.0) == 0.75_a);
                }
            }
        }
    }
}


SCENARIO("1D tent reconstruction")
{
    GIVEN("A continuous tent filter of radius 1.")
    {
        auto f = [](double x, double radius)
        {
            if (std::abs(x) < radius)
            {
                return 1.0 - (std::abs(x) / radius);
            }
            else
            {
                return 0.0;
            }
        };

        focg::Filter tent{
            std::bind(f, std::placeholders::_1, 1.0),
            1.0,
        };

        GIVEN("a discrete step sequence")
        {
            std::vector<double> step(10, 0.f);
            for (std::size_t i = 0;  i != 10; ++i)
            {
                step.push_back(1.0);
            }

            WHEN("The sequence is reconstructed at discrete values")
            {
                THEN("It matches expected values")
                {
                    // 10 is the middle point of the sequence (i.e. the first value 1)
                    CHECK(focg::reconstruct1D(step, tent,  9.0) == 0._a);
                    CHECK(focg::reconstruct1D(step, tent,  9.5) == 0.5_a);
                    CHECK(focg::reconstruct1D(step, tent,  9.8) == 0.8_a);
                    CHECK(focg::reconstruct1D(step, tent, 10.0) == 1.0_a);
                    CHECK(focg::reconstruct1D(step, tent, 15.2) == 1.0_a);
                }
            }
        }
    }
}

/***************************************************************************
 *            tests/test_scaling_adapters.cpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne Reasoning.
 *
 *  Ariadne Reasoning is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne Reasoning is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne Reasoning. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file tests/test_scaling_adapters.cpp
 *  @brief Verify per-location scaling parity and lazy materialization.
 */

#include "hybrid/hybrid_grid.hpp"
#include "logging/test_runner.hpp"
#include "reasoning/scaling_adapters.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class LocationDependentPolicy : public HybridScalingsInterface {
  public:
    LocationDependentPolicy* clone() const override {
        return new LocationDependentPolicy(*this);
    }

    ExactDouble scaling(const DiscreteLocation& location, const RealVariable& variable) const override {
        if (variable.name() == RealVariable("x").name()) {
            return location == DiscreteLocation(1) ? 2.0_x : 5.0_x;
        }
        if (variable.name() == RealVariable("y").name()) {
            return 3.0_x;
        }
        return 1.0_x;
    }

    Void _write(OutputStream& os) const override {
        os << "LocationDependentPolicy";
    }
};

class TestScalingAdapters : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_hybrid_scalings_match_legacy_and_are_lazy() {
        const DiscreteLocation first(1), second(2), absent(3);
        const RealVariable x("x"), y("y"), z("z");
        MonolithicHybridSpace spaces;
        spaces.new_location(first, RealSpace({x, z}));
        spaces.new_location(second, RealSpace({y, x}));
        const HybridSpace space(spaces);
        const HybridScalings policy(LocationDependentPolicy{});
        const HybridGrid legacy(space, policy);

        Hybrid<Scalings> generic = make_hybrid_scalings(space, policy);
        ARIADNE_TESTING_PROVE(generic.has_location(first));
        ARIADNE_TESTING_PROVE(generic.has_location(second));
        ARIADNE_TESTING_PROVE(not generic.has_location(absent));
        ARIADNE_TESTING_PROVE(not generic.has_value(first));
        ARIADNE_TESTING_PROVE(not generic.has_value(second));
        ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 0u);

        const Scalings& first_scalings = generic[first];
        ARIADNE_TESTING_EXPECT_EQUAL(first_scalings.scaling(x), 2.0_x);
        ARIADNE_TESTING_EXPECT_EQUAL(first_scalings.scaling(z), 1.0_x);
        ARIADNE_TESTING_PROVE(first_scalings.grid(space[first]) == legacy[first]);
        ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 1u);
        ARIADNE_TESTING_PROVE(not generic.has_value(second));

        const Scalings& second_scalings = generic[second];
        ARIADNE_TESTING_EXPECT_EQUAL(second_scalings.scaling(y), 3.0_x);
        ARIADNE_TESTING_EXPECT_EQUAL(second_scalings.scaling(x), 5.0_x);
        ARIADNE_TESTING_PROVE(second_scalings.grid(space[second]) == legacy[second]);
        ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 2u);

        const Hybrid<Scalings>& read_only = generic;
        ARIADNE_TESTING_EXPECT_EQUAL(read_only[first].scaling(x), 2.0_x);
        ARIADNE_TESTING_EXPECT_EQUAL(read_only[second].scaling(x), 5.0_x);
        ARIADNE_TESTING_EXPECT_EQUAL(read_only.materialized_locations().size(), 2u);
        ARIADNE_TESTING_EXPECT_EXCEPTION(generic[absent]);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_scaling_adapters");
    ARIADNE_TESTING_ADD_TEST(TestScalingAdapters, test_hybrid_scalings_match_legacy_and_are_lazy);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

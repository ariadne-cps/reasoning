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
 *  @brief Verify per-location scaling values and lazy grid materialization.
 */

#include "logging/test_runner.hpp"
#include "reasoning/hybrid_grid_value.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

namespace {

Hybrid<Scalings> make_location_dependent_scalings(const HybridSpace& space,
                                                  const RealVariable& x, const RealVariable& y,
                                                  const DiscreteLocation& first) {
    return Hybrid<Scalings>(space,
                            [x, y, first](const DiscreteLocation& location, const RealSpace& real_space) {
                                Map<RealVariable, ExactDouble> entries;
                                for (SizeType index = 0; index < real_space.dimension(); ++index) {
                                    const RealVariable& variable = real_space[index];
                                    if (variable.name() == x.name())
                                        entries.insert(variable, location == first ? 2.0_x : 5.0_x);
                                    else if (variable.name() == y.name())
                                        entries.insert(variable, 3.0_x);
                                }
                                return Scalings(1.0_x, entries);
                            });
}

} // namespace

class TestScalingAdapters : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_hybrid_scalings_are_lazy_and_location_dependent() {
        const DiscreteLocation first(1), second(2), absent(3);
        const RealVariable x("x"), y("y"), z("z");
        MonolithicHybridSpace spaces;
        spaces.new_location(first, RealSpace({x, z}));
        spaces.new_location(second, RealSpace({y, x}));
        const HybridSpace space(spaces);
        Hybrid<Scalings> generic = make_location_dependent_scalings(space, x, y, first);

        ARIADNE_TESTING_PROVE(generic.has_location(first));
        ARIADNE_TESTING_PROVE(generic.has_location(second));
        ARIADNE_TESTING_PROVE(not generic.has_location(absent));
        ARIADNE_TESTING_PROVE(not generic.has_value(first));
        ARIADNE_TESTING_PROVE(not generic.has_value(second));
        ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 0u);

        const Scalings& first_scalings = generic[first];
        ARIADNE_TESTING_EXPECT_EQUAL(first_scalings.scaling(x), 2.0_x);
        ARIADNE_TESTING_EXPECT_EQUAL(first_scalings.scaling(z), 1.0_x);
        ARIADNE_TESTING_PROVE(grid_from_scalings(first_scalings, space[first]) ==
                              Grid(Vector<ExactDouble>{2.0_x, 1.0_x}));

        ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 1u);
        ARIADNE_TESTING_PROVE(not generic.has_value(second));

        const Scalings& second_scalings = generic[second];
        ARIADNE_TESTING_EXPECT_EQUAL(second_scalings.scaling(y), 3.0_x);
        ARIADNE_TESTING_EXPECT_EQUAL(second_scalings.scaling(x), 5.0_x);
        ARIADNE_TESTING_PROVE(grid_from_scalings(second_scalings, space[second]) ==
                              Grid(Vector<ExactDouble>{3.0_x, 5.0_x}));

        ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 2u);

        const Hybrid<Scalings>& read_only = generic;
        ARIADNE_TESTING_EXPECT_EQUAL(read_only[first].scaling(x), 2.0_x);
        ARIADNE_TESTING_EXPECT_EQUAL(read_only[second].scaling(x), 5.0_x);
        ARIADNE_TESTING_EXPECT_EQUAL(read_only.materialized_locations().size(), 2u);
        ARIADNE_TESTING_EXPECT_EXCEPTION(generic[absent]);

        Hybrid<Grid> generic_grids = make_hybrid_grids(
            space, make_location_dependent_scalings(space, x, y, first));
        ARIADNE_TESTING_PROVE(not generic_grids.has_value(first));
        ARIADNE_TESTING_PROVE(not generic_grids.has_value(second));
        ARIADNE_TESTING_PROVE(generic_grids[first] == Grid(Vector<ExactDouble>{2.0_x, 1.0_x}));
        ARIADNE_TESTING_PROVE(not generic_grids.has_value(second));
        ARIADNE_TESTING_PROVE(generic_grids[second] == Grid(Vector<ExactDouble>{3.0_x, 5.0_x}));
        ARIADNE_TESTING_EXPECT_EQUAL(generic_grids.size(), 2u);
    }

    Void test_grid_factories_from_generic_scalings() {
        const DiscreteLocation first(1), second(2), absent(9);
        const RealVariable x("x"), y("y"), z("z");
        MonolithicHybridSpace locations;
        locations.new_location(first, RealSpace({x}));
        locations.new_location(second, RealSpace({y, x}));
        const HybridSpace space(locations);
        Hybrid<Scalings> scalings = make_location_dependent_scalings(space, x, y, first);
        Hybrid<Grid> generic = make_hybrid_grids(space, scalings);
        ARIADNE_TESTING_PROVE(not generic.has_value(first));
        ARIADNE_TESTING_PROVE(not generic.has_value(second));
        ARIADNE_TESTING_PROVE(generic[first] == Grid(Vector<ExactDouble>{2.0_x}));
        ARIADNE_TESTING_PROVE(not generic.has_value(second));
        ARIADNE_TESTING_PROVE(generic[second] == Grid(Vector<ExactDouble>{3.0_x, 5.0_x}));
        ARIADNE_TESTING_PROVE(grid_from_scalings(scalings[second], RealSpace({x, y})) ==
                              Grid(Vector<ExactDouble>{5.0_x, 3.0_x}));
        ARIADNE_TESTING_EXPECT_EXCEPTION(generic[absent]);

        const Map<RealVariable, ExactDouble> entries{
            {x, 2.0_x},
            {y, 3.0_x}
        };
        const Scalings fallback(4.0_x, entries);
        const RealSpace auxiliary({z, y, x});
        const Grid expected(Vector<ExactDouble>{4.0_x, 3.0_x, 2.0_x});
        ARIADNE_TESTING_PROVE(grid_from_scalings(fallback, auxiliary) == expected);
        ARIADNE_TESTING_EXPECT_EQUAL(grid_from_scalings(fallback, RealSpace()).dimension(), 0u);
    }

    Void test_grid_materialization_from_euclidean_scalings() {
        const RealVariable x("x"), y("y");
        const RealSpace reordered({y, x});
        const Map<RealVariable, ExactDouble> entries{
            {x, 2.0_x}
        };
        const Scalings named(3.0_x, entries);
        const Grid named_grid = grid_from_scalings(named, reordered);
        ARIADNE_TESTING_EXPECT_EQUAL(named_grid.dimension(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(named_grid.lengths()[0].get_d(), 3.0);
        ARIADNE_TESTING_EXPECT_EQUAL(named_grid.lengths()[1].get_d(), 2.0);

        Scalings uniform(ApproximateDouble(2.0));
        const RealSpace natural({x, y});
        const Grid uniform_grid = grid_from_scalings(uniform, natural);
        ARIADNE_TESTING_EXPECT_EQUAL(uniform_grid.dimension(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(uniform_grid.lengths()[0].get_d(), 2.0);
        ARIADNE_TESTING_EXPECT_EQUAL(uniform_grid.lengths()[1].get_d(), 2.0);
        uniform.set_scaling(x, ApproximateDouble(4.0));
        const Grid modified = grid_from_scalings(uniform, natural);
        ARIADNE_TESTING_EXPECT_EQUAL(modified.lengths()[0].get_d(), 4.0);
        ARIADNE_TESTING_EXPECT_EQUAL(modified.lengths()[1].get_d(), 2.0);

        const Scalings empty(1.0_x, Map<RealVariable, ExactDouble>{});
        ARIADNE_TESTING_EXPECT_EQUAL(grid_from_scalings(empty, RealSpace()).dimension(), 0u);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_scaling_adapters");
    ARIADNE_TESTING_ADD_TEST(TestScalingAdapters, test_hybrid_scalings_are_lazy_and_location_dependent);
    ARIADNE_TESTING_ADD_TEST(TestScalingAdapters, test_grid_materialization_from_euclidean_scalings);
    ARIADNE_TESTING_ADD_TEST(TestScalingAdapters, test_grid_factories_from_generic_scalings);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

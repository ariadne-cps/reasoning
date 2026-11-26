/***************************************************************************
 *            tests/test_grid_adapters.cpp
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

/** @file tests/test_grid_adapters.cpp
 *  @brief Verify generic grid wrappers against legacy labelled and hybrid grids.
 */

#include <memory>

#include "labelled/labelled_grid.hpp"
#include "logging/test_runner.hpp"
#include "paving/robdd_paving.hpp"
#include "reasoning/hybrid_grid_compat.hpp"
#include "reasoning/labelled_paving_value.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestGridAdapters : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_labelled_grid_matches_legacy() {
        const RealVariable x("x"), y("y");
        const RealSpace space({y, x});
        const Grid grid(2u);
        const LabelledGrid legacy(grid, space);
        const LabelledValue<Grid> generic = make_labelled_grid(grid, space);
        ARIADNE_TESTING_EXPECT_EQUAL(generic.space(), legacy.space());
        ARIADNE_TESTING_PROVE(generic.value() == legacy.euclidean_grid());
        ARIADNE_TESTING_EXPECT_EQUAL(generic.value().dimension(), 2u);
        ARIADNE_TESTING_EXPECT_EXCEPTION(make_labelled_grid(Grid(1u), space));
    }

    Void test_legacy_location_scalings_preserve_coordinate_order() {
        const DiscreteLocation first(1), absent(9);
        const RealVariable x("x"), y("y"), z("z");
        const Map<Identifier, ExactDouble> named{
            {x.name(), 2.0_x}, {y.name(), 3.0_x}
        };
        const HybridScalings policy{SimpleHybridScalings(4.0_x, named)};

        const Grid permuted = grid_from_scalings(policy, first, RealSpace({y, z, x}));
        ARIADNE_TESTING_EXPECT_EQUAL(permuted.dimension(), 3u);
        ARIADNE_TESTING_PROVE(permuted == Grid(Vector<ExactDouble>{3.0_x, 4.0_x, 2.0_x}));
        const Grid reversed = grid_from_scalings(policy, first, RealSpace({x, y}));
        ARIADNE_TESTING_PROVE(reversed == Grid(Vector<ExactDouble>{2.0_x, 3.0_x}));

        MonolithicHybridSpace known;
        known.new_location(first, RealSpace({x, y}));
        const HybridSpace space(known);
        Hybrid<Grid> generic = make_hybrid_grids(space, policy);
        ARIADNE_TESTING_PROVE(generic.has_location(first));
        ARIADNE_TESTING_PROVE(not generic.has_location(absent));
        ARIADNE_TESTING_PROVE(not generic.has_value(first));
        ARIADNE_TESTING_PROVE(generic[first] == reversed);
        ARIADNE_TESTING_PROVE(not generic.has_value(absent));
        ARIADNE_TESTING_EXPECT_EXCEPTION(generic[absent]);
    }

    Void test_labelled_pavings_preserve_tree_and_robdd() {
        const RealVariable x("x"), y("y");
        const RealSpace space({y, x});
        const Grid grid(2u);
        const GridCell cell(grid, 0u, BinaryWord());

        BinaryTreePaving tree(grid);
        tree.adjoin(cell);
        const Paving tree_paving(new BinaryTreePaving(tree));
        const LabelledPaving legacy_tree(tree_paving, space);
        const LabelledValue<Paving> generic_tree = make_labelled_paving(tree_paving, space);
        ARIADNE_TESTING_EXPECT_EQUAL(generic_tree.space(), legacy_tree.space());
        ARIADNE_TESTING_EXPECT_EQUAL(generic_tree.value().dimension(), 2u);
        ARIADNE_TESTING_PROVE(generic_tree.value().superset(cell));
        ARIADNE_TESTING_PROVE(legacy_tree.euclidean_set().superset(cell));
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const BinaryTreePaving*>(
                &static_cast<const PavingInterface&>(generic_tree.value())) != nullptr);

        const auto manager = std::make_shared<RobddManager>();
        RobddPaving robdd(manager, grid);
        robdd.adjoin(cell);
        const Paving robdd_paving(new RobddPaving(robdd));
        const LabelledPaving legacy_robdd(robdd_paving, space);
        const LabelledValue<Paving> generic_robdd = make_labelled_paving(robdd_paving, space);
        ARIADNE_TESTING_EXPECT_EQUAL(generic_robdd.space(), legacy_robdd.space());
        ARIADNE_TESTING_PROVE(generic_robdd.value().superset(cell));
        ARIADNE_TESTING_PROVE(legacy_robdd.euclidean_set().superset(cell));
        const LabelledValue<Paving> copied(generic_robdd);
        ARIADNE_TESTING_PROVE(copied.value().superset(cell));
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const RobddPaving*>(
                &static_cast<const PavingInterface&>(copied.value())) != nullptr);

        ARIADNE_TESTING_EXPECT_EXCEPTION(
            make_labelled_paving(tree_paving, RealSpace({x})));
    }

    Void test_hybrid_grids_match_legacy_and_are_lazy() {
        const DiscreteLocation first(1), second(2);
        const RealVariable x("x"), y("y");
        MonolithicHybridSpace spaces;
        spaces.new_location(first, RealSpace({x}));
        spaces.new_location(second, RealSpace({y, x}));
        const HybridSpace space(spaces);
        const Map<Identifier, ExactDouble> scales{
            {x.name(), 2.0_x},
            {y.name(), 3.0_x}
        };
        const HybridScalings scaling(scales);
        const HybridGrid legacy(space, scaling);
        Hybrid<Grid> generic = make_hybrid_grids(space, scaling);
        ARIADNE_TESTING_PROVE(generic.has_location(first));
        ARIADNE_TESTING_PROVE(generic.has_location(second));
        ARIADNE_TESTING_PROVE(not generic.has_value(first));
        ARIADNE_TESTING_PROVE(not generic.has_value(second));

        const Grid& one = generic[first];
        ARIADNE_TESTING_PROVE(one == legacy[first]);
        ARIADNE_TESTING_EXPECT_EQUAL(one.dimension(), 1u);
        ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 1u);
        ARIADNE_TESTING_PROVE(not generic.has_value(second));

        const Grid& second_grid = generic[second];
        ARIADNE_TESTING_PROVE(second_grid == legacy[second]);
        ARIADNE_TESTING_EXPECT_EQUAL(second_grid.dimension(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 2u);

        const Hybrid<Grid>& read_only = generic;
        ARIADNE_TESTING_PROVE(read_only[first] == one);
        ARIADNE_TESTING_PROVE(read_only[second] == second_grid);
        ARIADNE_TESTING_EXPECT_EQUAL(read_only.materialized_locations().size(), 2u);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_grid_adapters");
    ARIADNE_TESTING_ADD_TEST(TestGridAdapters, test_labelled_grid_matches_legacy);
    ARIADNE_TESTING_ADD_TEST(TestGridAdapters, test_labelled_pavings_preserve_tree_and_robdd);
    ARIADNE_TESTING_ADD_TEST(TestGridAdapters, test_hybrid_grids_match_legacy_and_are_lazy);
    ARIADNE_TESTING_ADD_TEST(TestGridAdapters, test_legacy_location_scalings_preserve_coordinate_order);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

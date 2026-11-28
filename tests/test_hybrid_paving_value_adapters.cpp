/***************************************************************************
 *            tests/test_hybrid_paving_value_adapters.cpp
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

/** @file tests/test_hybrid_paving_value_adapters.cpp
 *  @brief Exercise lazy hybrid paving values with tree and ROBDD storage.
 */

#include <memory>

#include "function/function.hpp"
#include "geometry/box.hpp"
#include "hybrid/hybrid_bounded_constraint_set.hpp"
#include "hybrid/hybrid_set.hpp"
#include "hybrid_scalings_test_util.hpp"
#include "logging/test_runner.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/robdd_paving.hpp"
#include "reasoning/hybrid_augmented_paving_value.hpp"
#include "reasoning/hybrid_grid_value.hpp"
#include "reasoning/hybrid_paving_value.hpp"
#include "system/hybrid_automaton.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestHybridPavingValueAdapters : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_tree_robdd_and_lazy_locations() {
        const DiscreteLocation first(1), second(2), absent(3);
        const RealVariable x("x"), y("y");
        MonolithicHybridSpace spaces;
        spaces.new_location(first, RealSpace({x}));
        spaces.new_location(second, RealSpace({y, x}));
        const HybridSpace space(spaces);
        const Map<Identifier, ExactDouble> scale_values{
            {x.name(), 2.0_x},
            {y.name(), 3.0_x}
        };

        for (Bool use_robdd: {false, true}) {
            auto manager = std::make_shared<RobddManager>();
            Nat backend_creations = 0u;
            auto create_backend = [manager, use_robdd, &backend_creations](
                                      const Grid& selected_grid) -> Paving {
                ++backend_creations;
                if (use_robdd)
                    return Paving(new RobddPaving(manager, selected_grid));
                return Paving(new BinaryTreePaving(selected_grid));
            };

            Hybrid<Grid> generic_grids = make_hybrid_grids(
                space, test_scalings_from_map(space, 1.0_x, scale_values));
            Hybrid<Paving> generic = make_hybrid_pavings(
                space, std::move(generic_grids), create_backend);

            ARIADNE_TESTING_PROVE(generic.has_location(first));
            ARIADNE_TESTING_PROVE(generic.has_location(second));
            ARIADNE_TESTING_PROVE(not generic.has_location(absent));
            ARIADNE_TESTING_PROVE(not generic.has_value(first));
            ARIADNE_TESTING_PROVE(not generic.has_value(second));
            ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 0u);
            ARIADNE_TESTING_EXPECT_EQUAL(backend_creations, 0u);

            const Grid grid_first(Vector<ExactDouble>{2.0_x});
            const GridCell cell_first(grid_first, 0u, BinaryWord());
            Paving& generic_first = generic[first];
            ARIADNE_TESTING_EXPECT_EQUAL(backend_creations, 1u);
            ARIADNE_TESTING_PROVE(generic_first.grid() == grid_first);
            ARIADNE_TESTING_PROVE(not generic.has_value(second));
            generic_first.adjoin(cell_first);
            ARIADNE_TESTING_PROVE(generic_first.superset(cell_first));

            const Grid grid_second(Vector<ExactDouble>{3.0_x, 2.0_x});
            const GridCell cell_second(grid_second, 0u, BinaryWord());
            Paving& generic_second = generic[second];
            ARIADNE_TESTING_EXPECT_EQUAL(backend_creations, 2u);
            ARIADNE_TESTING_PROVE(generic_second.grid() == grid_second);
            generic_second.adjoin(cell_second);
            ARIADNE_TESTING_PROVE(generic_second.superset(cell_second));
            ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 2u);

            const Hybrid<Paving>& read_only = generic;
            ARIADNE_TESTING_PROVE(read_only[first].superset(cell_first));
            ARIADNE_TESTING_PROVE(read_only[second].superset(cell_second));
            ARIADNE_TESTING_EXPECT_EQUAL(read_only.materialized_locations().size(), 2u);
            ARIADNE_TESTING_EXPECT_EXCEPTION(generic[absent]);

            const Hybrid<Paving> copy(generic);
            ARIADNE_TESTING_PROVE(copy[first].superset(cell_first));
            ARIADNE_TESTING_PROVE(copy[second].superset(cell_second));
            if (use_robdd) {
                ARIADNE_TESTING_PROVE(dynamic_cast<const RobddPaving*>(
                                          &static_cast<const PavingInterface&>(copy[first])) != nullptr);
                ARIADNE_TESTING_PROVE(dynamic_cast<const RobddPaving*>(
                                          &static_cast<const PavingInterface&>(copy[second])) != nullptr);
            } else {
                ARIADNE_TESTING_PROVE(dynamic_cast<const BinaryTreePaving*>(
                                          &static_cast<const PavingInterface&>(copy[first])) != nullptr);
                ARIADNE_TESTING_PROVE(dynamic_cast<const BinaryTreePaving*>(
                                          &static_cast<const PavingInterface&>(copy[second])) != nullptr);
            }
        }
    }

    Void test_extend_auxiliary_preserves_lazy_backend() {
        const RealVariable x("x"), a("a");
        const StringVariable q("q");
        const DiscreteLocation first({q | "1"}), second({q | "2"});
        HybridAutomaton automaton;
        ARIADNE_TESTING_EXPECT_NO_THROW(automaton.new_mode(
            first, List<RealAssignment>{let(a) = x + 1},
            List<DottedRealAssignment>{dot(x) = -x}));
        ARIADNE_TESTING_EXPECT_NO_THROW(automaton.new_mode(
            second, List<RealAssignment>{let(a) = x + 2},
            List<DottedRealAssignment>{dot(x) = -x}));

        const HybridSpace state_space = automaton.state_space();
        const HybridSpace expanded_space = automaton.state_auxiliary_space();
        const auto unit_scalings = [](const DiscreteLocation&, const RealSpace&) {
            return Scalings(1.0_x, Map<RealVariable, ExactDouble>{});
        };
        auto make_input = [&](const std::function<Paving(const Grid&)>& backend) {
            Hybrid<Paving> result = make_hybrid_pavings(
                state_space, make_hybrid_grids(state_space, Hybrid<Scalings>(state_space, unit_scalings)),
                backend);
            result[first].adjoin(GridCell(Grid(1u), 0u, BinaryWord()));
            return result;
        };

        for (Bool use_robdd: {false, true}) {
            auto manager = std::make_shared<RobddManager>();
            std::function<Paving(const Grid&)> backend =
                [manager, use_robdd](const Grid& selected) -> Paving {
                if (use_robdd)
                    return Paving(new RobddPaving(manager, selected));
                return Paving(new BinaryTreePaving(selected));
            };

            Hybrid<Paving> generic = make_input(backend);
            ARIADNE_TESTING_PROVE(not generic.has_value(second));

            Hybrid<Paving> expanded = extend_auxiliary(
                generic, Hybrid<Scalings>(expanded_space, unit_scalings),
                backend, automaton);
            Hybrid<Paving> replica = extend_auxiliary(
                make_input(backend), Hybrid<Scalings>(expanded_space, unit_scalings),
                backend, automaton);
            ARIADNE_TESTING_PROVE(expanded.has_location(second));
            ARIADNE_TESTING_PROVE(expanded.has_value(first));
            ARIADNE_TESTING_PROVE(not expanded.has_value(second));
            ARIADNE_TESTING_PROVE(not generic.has_value(second));
            ARIADNE_TESTING_EXPECT_EQUAL(expanded.space()[first].dimension(), 2u);

            const Hybrid<Paving>& constant_expanded = expanded;
            const Paving& actual = constant_expanded[first];
            const Paving& expected = replica[first];
            ARIADNE_TESTING_PROVE(actual.grid() == expected.grid());
            for (const auto& selected_cell: actual)
                ARIADNE_TESTING_PROVE(expected.superset(selected_cell));
            for (const auto& selected_cell: expected)
                ARIADNE_TESTING_PROVE(actual.superset(selected_cell));
            ARIADNE_TESTING_PROVE(not actual.is_empty());

            const Hybrid<Paving> copied(expanded);
            ARIADNE_TESTING_PROVE(not copied.has_value(second));
            const Paving& snapshot = copied[first];
            for (const auto& selected_cell: expected)
                ARIADNE_TESTING_PROVE(snapshot.superset(selected_cell));
            if (use_robdd) {
                ARIADNE_TESTING_PROVE(dynamic_cast<const RobddPaving*>(
                                          &static_cast<const PavingInterface&>(snapshot)) != nullptr);
            } else {
                ARIADNE_TESTING_PROVE(dynamic_cast<const BinaryTreePaving*>(
                                          &static_cast<const PavingInterface&>(snapshot)) != nullptr);
            }
        }
    }

    Void test_auxiliary_extension_preserves_named_grid_scalings() {
        const RealVariable x("x"), a("a");
        const StringVariable q("q");
        const DiscreteLocation first({q | "1"}), second({q | "2"});
        HybridAutomaton automaton;
        ARIADNE_TESTING_EXPECT_NO_THROW(automaton.new_mode(
            first, List<RealAssignment>{let(a) = x + 1},
            List<DottedRealAssignment>{dot(x) = -x}));
        ARIADNE_TESTING_EXPECT_NO_THROW(automaton.new_mode(
            second, List<RealAssignment>{let(a) = x + 2},
            List<DottedRealAssignment>{dot(x) = -x}));

        const Map<Identifier, ExactDouble> named{
            {x.name(), 2.0_x},
            {a.name(), 3.0_x}
        };
        const HybridSpace state_space = automaton.state_space();
        const HybridSpace expanded_space = automaton.state_auxiliary_space();
        auto named_scalings = [&named](const HybridSpace& space) {
            return Hybrid<Scalings>(space, [&named](const DiscreteLocation&, const RealSpace& rs) {
                Map<RealVariable, ExactDouble> entries;
                for (SizeType index = 0; index < rs.dimension(); ++index)
                    if (named.has_key(rs[index].name()))
                        entries.insert(rs[index], named[rs[index].name()]);
                return Scalings(1.0_x, entries);
            });
        };
        Hybrid<Scalings> state_scalings = named_scalings(state_space);
        Hybrid<Scalings> expanded_scalings = named_scalings(expanded_space);
        Hybrid<Grid> state_grids = make_hybrid_grids(state_space, state_scalings);

        for (Bool use_robdd: {false, true}) {
            auto manager = std::make_shared<RobddManager>();
            std::function<Paving(const Grid&)> backend =
                [manager, use_robdd](const Grid& selected) -> Paving {
                if (use_robdd)
                    return Paving(new RobddPaving(manager, selected));
                return Paving(new BinaryTreePaving(selected));
            };

            Hybrid<Paving> generic = make_hybrid_pavings(state_space, state_grids, backend);
            generic[first].adjoin(GridCell(state_grids[first], 0u, BinaryWord()));
            ARIADNE_TESTING_PROVE(not generic.has_value(second));
            const Hybrid<Paving> expanded = extend_auxiliary(
                generic, expanded_scalings, backend, automaton);
            ARIADNE_TESTING_PROVE(expanded.has_value(first));
            ARIADNE_TESTING_PROVE(not expanded.has_value(second));

            const RealSpace expanded_coordinates = expanded_space[first];
            const Grid expected_grid =
                grid_from_scalings(expanded_scalings[first], expanded_coordinates);
            ARIADNE_TESTING_PROVE(expanded[first].grid() == expected_grid);
            ARIADNE_TESTING_EXPECT_EQUAL(expected_grid.dimension(), 2u);
            for (SizeType axis = 0; axis < expanded_coordinates.dimension(); ++axis) {
                ARIADNE_TESTING_EXPECT_EQUAL(
                    expected_grid.lengths()[axis],
                    named[expanded_coordinates[axis].name()]);
            }
            ARIADNE_TESTING_PROVE(not expanded[first].is_empty());
            const PavingInterface& selected =
                static_cast<const PavingInterface&>(expanded[first]);
            if (use_robdd)
                ARIADNE_TESTING_PROVE(dynamic_cast<const RobddPaving*>(&selected) != nullptr);
            else
                ARIADNE_TESTING_PROVE(dynamic_cast<const BinaryTreePaving*>(&selected) != nullptr);
        }
    }

    Void test_set_operations_without_unwanted_materialization() {
        const DiscreteLocation first(1), second(2), third(3), absent(4);
        const RealVariable x("x");
        MonolithicHybridSpace spaces;
        spaces.new_location(first, RealSpace({x}));
        spaces.new_location(second, RealSpace({x}));
        spaces.new_location(third, RealSpace({x}));
        const HybridSpace space(spaces);
        const Grid grid(1u);
        const GridCell cell_left(grid, 0u, BinaryWord("0"));
        const GridCell cell_right(grid, 0u, BinaryWord("1"));

        for (Bool use_robdd: {false, true}) {
            auto manager = std::make_shared<RobddManager>();
            std::function<Paving(const Grid&)> backend =
                [manager, use_robdd](const Grid& selected) -> Paving {
                if (use_robdd)
                    return Paving(new RobddPaving(manager, selected));
                return Paving(new BinaryTreePaving(selected));
            };
            const auto make_generic = [&]() {
                return make_hybrid_pavings(
                    space, make_hybrid_grids(space, Hybrid<Scalings>(space, [](const DiscreteLocation&, const RealSpace&) {
                                                 return Scalings(1.0_x, Map<RealVariable, ExactDouble>{});
                                             })),
                    backend);
            };
            Hybrid<Paving> original_left = make_generic();
            Hybrid<Paving> original_right = make_generic();
            original_left[first].adjoin(cell_left);
            original_left[second].adjoin(cell_right);
            original_right[first].adjoin(cell_right);
            original_right[third].adjoin(cell_left);

            // Comparisons must not force lazy construction.
            ARIADNE_TESTING_PROVE(not subset(original_left, original_right));
            ARIADNE_TESTING_PROVE(not intersect(original_left, original_right));
            ARIADNE_TESTING_EXPECT_EQUAL(original_left.size(), 2u);
            ARIADNE_TESTING_EXPECT_EQUAL(original_right.size(), 2u);
            ARIADNE_TESTING_PROVE(not original_left.has_value(third));
            ARIADNE_TESTING_PROVE(not original_right.has_value(second));
            ARIADNE_TESTING_PROVE(not original_left.has_location(absent));

            Hybrid<Paving> joined = original_left;
            adjoin(joined, original_right);
            ARIADNE_TESTING_EXPECT_EQUAL(joined.size(), 3u);
            ARIADNE_TESTING_PROVE(joined[first].superset(cell_left));
            ARIADNE_TESTING_PROVE(joined[first].superset(cell_right));
            ARIADNE_TESTING_PROVE(subset(original_right, joined));
            ARIADNE_TESTING_PROVE(intersect(joined, original_right));

            Hybrid<Paving> difference = original_left;
            remove(difference, original_right);
            ARIADNE_TESTING_PROVE(difference.has_value(third));
            ARIADNE_TESTING_PROVE(difference[third].is_empty());
            ARIADNE_TESTING_PROVE(difference[first].superset(cell_left));
            ARIADNE_TESTING_PROVE(not difference[first].superset(cell_right));

            Hybrid<Paving> restricted = original_left;
            restrict(restricted, original_right);
            ARIADNE_TESTING_PROVE(restricted[first].is_empty());
            ARIADNE_TESTING_PROVE(restricted[second].is_empty());
            ARIADNE_TESTING_PROVE(not restricted.has_value(third));
            ARIADNE_TESTING_PROVE(subset(restricted, original_right));
            ARIADNE_TESTING_PROVE(not intersect(restricted, original_right));

            // Neither source acquires a materialized location as a side effect.
            ARIADNE_TESTING_EXPECT_EQUAL(original_left.size(), 2u);
            ARIADNE_TESTING_EXPECT_EQUAL(original_right.size(), 2u);
            ARIADNE_TESTING_PROVE(not original_left.has_value(third));
            ARIADNE_TESTING_PROVE(not original_right.has_value(second));
        }
    }

    Void test_rejects_empty_or_wrong_grid_backend() {
        const DiscreteLocation first(1);
        const RealVariable x("x");
        MonolithicHybridSpace spaces;
        spaces.new_location(first, RealSpace({x}));
        const HybridSpace space(spaces);

        Hybrid<Grid> grids(space,
                           [](const DiscreteLocation&, const RealSpace& coordinates) {
                               return Grid(coordinates.dimension());
                           });
        ARIADNE_TESTING_EXPECT_EXCEPTION(
            make_hybrid_pavings(space, grids, std::function<Paving(const Grid&)>{}));

        Hybrid<Paving> invalid = make_hybrid_pavings(
            space, grids, [](const Grid&) {
                return Paving(new BinaryTreePaving(Grid(2u)));
            });
        ARIADNE_TESTING_PROVE(not invalid.has_value(first));
        ARIADNE_TESTING_EXPECT_EXCEPTION(invalid[first]);
        ARIADNE_TESTING_PROVE(not invalid.has_value(first));
    }

    Void test_generic_approximation_and_augmented_value() {
        const DiscreteLocation first(1), second(2);
        const RealVariable x("x"), y("y");
        MonolithicHybridSpace spaces;
        spaces.new_location(first, RealSpace({x}));
        spaces.new_location(second, RealSpace({y, x}));
        const HybridSpace space(spaces);
        const Map<Identifier, ExactDouble> scale_values{
            {x.name(), 2.0_x},
            {y.name(), 3.0_x}
        };
        auto create_tree = [](const Grid& g) { return Paving(new BinaryTreePaving(g)); };
        auto generic_grids = [&] { return make_hybrid_grids(space, test_scalings_from_map(space, 1.0_x, scale_values)); };

        // Outer approximation of a hybrid compact set: only its location materializes.
        const HybridBoundedConstraintSet compact_set(first, {0.5_dec <= x <= 1.5_dec});
        Hybrid<Paving> generic = make_hybrid_pavings(space, generic_grids(), create_tree);
        adjoin_outer_approximation(generic, compact_set, 2u);
        ARIADNE_TESTING_PROVE(generic[first].size() > 0u);
        ARIADNE_TESTING_PROVE(not generic.has_value(second));

        // Two independently built values agree slice by slice.
        Hybrid<Paving> replica = make_hybrid_pavings(space, generic_grids(), create_tree);
        adjoin_outer_approximation(replica, compact_set, 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(generic[first].size(), replica[first].size());
        ARIADNE_TESTING_PROVE(generic[first].subset(static_cast<const PavingInterface&>(replica[first])));
        ARIADNE_TESTING_PROVE(replica[first].subset(static_cast<const PavingInterface&>(generic[first])));

        // Lower approximation matches an independent replica.
        Hybrid<Paving> generic_lower = make_hybrid_pavings(space, generic_grids(), create_tree);
        adjoin_lower_approximation(generic_lower, compact_set, 4u, 6u);
        ARIADNE_TESTING_PROVE(generic_lower[first].size() > 0u);
        Hybrid<Paving> replica_lower = make_hybrid_pavings(space, generic_grids(), create_tree);
        adjoin_lower_approximation(replica_lower, compact_set, 4u, 6u);
        ARIADNE_TESTING_EXPECT_EQUAL(generic_lower[first].size(), replica_lower[first].size());
        ARIADNE_TESTING_PROVE(generic_lower[first].subset(static_cast<const PavingInterface&>(replica_lower[first])));
        ARIADNE_TESTING_PROVE(replica_lower[first].subset(static_cast<const PavingInterface&>(generic_lower[first])));

        // Extent restriction matches as well.
        restrict_to_extent(generic, 3u);
        restrict_to_extent(replica, 3u);
        ARIADNE_TESTING_EXPECT_EQUAL(generic[first].size(), replica[first].size());

        // Outer approximation of exact boxes on both locations.
        HybridExactBoxes boxes;
        boxes.insert(first, RealSpace({
                                x
        }),
                     ExactBoxType({{-1, 1}}));
        boxes.insert(second, RealSpace({
                                 y, x
        }),
                     ExactBoxType({{-2, 2}, {-2, 2}}));
        Hybrid<Paving> generic_boxes = make_hybrid_pavings(space, generic_grids(), create_tree);
        adjoin_outer_approximation(generic_boxes, boxes, 2u);
        Hybrid<Paving> replica_boxes = make_hybrid_pavings(space, generic_grids(), create_tree);
        adjoin_outer_approximation(replica_boxes, boxes, 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(generic_boxes[first].size(), replica_boxes[first].size());
        ARIADNE_TESTING_EXPECT_EQUAL(generic_boxes[second].size(), replica_boxes[second].size());
        ARIADNE_TESTING_PROVE(generic_boxes[first].subset(static_cast<const PavingInterface&>(replica_boxes[first])));
        ARIADNE_TESTING_PROVE(replica_boxes[first].subset(static_cast<const PavingInterface&>(generic_boxes[first])));

        // Augmented values delegate to the state paving per location.
        HybridAutomaton system;
        system.new_mode(first, {dot(x) = -x});
        system.new_mode(second, {dot(y) = -y, dot(x) = -x});
        HybridAugmentedPaving generic_aug = make_hybrid_augmented_paving(generic_grids(), system, create_tree);
        adjoin_outer_approximation(generic_aug, compact_set, 2u);
        ARIADNE_TESTING_PROVE(generic_aug[first].size() > 0u);
        HybridAugmentedPaving replica_aug = make_hybrid_augmented_paving(generic_grids(), system, create_tree);
        adjoin_outer_approximation(replica_aug, compact_set, 2u);
        ARIADNE_TESTING_PROVE(subset(generic_aug, replica_aug));
        ARIADNE_TESTING_PROVE(subset(replica_aug, generic_aug));
    }

    Void test_mince_recombine_is_empty_and_inner_approximation() {
        const DiscreteLocation first(1), second(2);
        const RealVariable x("x"), y("y");
        MonolithicHybridSpace spaces;
        spaces.new_location(first, RealSpace({x}));
        spaces.new_location(second, RealSpace({y, x}));
        const HybridSpace space(spaces);
        const Map<Identifier, ExactDouble> scale_values{
            {x.name(), 2.0_x},
            {y.name(), 3.0_x}
        };
        auto create_tree = [](const Grid& g) { return Paving(new BinaryTreePaving(g)); };
        auto generic_grids = [&] { return make_hybrid_grids(space, test_scalings_from_map(space, 1.0_x, scale_values)); };

        const HybridBoundedConstraintSet compact_set(first, {0.5_dec <= x <= 1.5_dec});
        const HybridBoundedConstraintSet wide_set(first, {-1.0_dec <= x <= 3.0_dec});

        // Factory inner approximation: only the set locations materialize.
        const Hybrid<Paving> generic_inner = inner_approximation(wide_set, generic_grids(), create_tree, 2u);
        ARIADNE_TESTING_PROVE(generic_inner.has_value(first));
        ARIADNE_TESTING_PROVE(not generic_inner.has_value(second));
        const Hybrid<Paving> replica_inner = inner_approximation(wide_set, generic_grids(), create_tree, 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(generic_inner[first].size(), replica_inner[first].size());
        ARIADNE_TESTING_PROVE(generic_inner[first].subset(static_cast<const PavingInterface&>(replica_inner[first])));
        ARIADNE_TESTING_PROVE(replica_inner[first].subset(static_cast<const PavingInterface&>(generic_inner[first])));
        ARIADNE_TESTING_PROVE(not is_empty(generic_inner));

        // adjoin_inner_approximation on existing pavings; fresh values are empty.
        Hybrid<Paving> generic = make_hybrid_pavings(space, generic_grids(), create_tree);
        ARIADNE_TESTING_PROVE(is_empty(generic));
        adjoin_inner_approximation(generic, wide_set, 2u);
        ARIADNE_TESTING_PROVE(not is_empty(generic));
        Hybrid<Paving> replica = make_hybrid_pavings(space, generic_grids(), create_tree);
        adjoin_inner_approximation(replica, wide_set, 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(generic[first].size(), replica[first].size());
        ARIADNE_TESTING_PROVE(generic[first].subset(static_cast<const PavingInterface&>(replica[first])));
        ARIADNE_TESTING_PROVE(replica[first].subset(static_cast<const PavingInterface&>(generic[first])));

        // mince/recombine parity on an outer approximation.
        Hybrid<Paving> generic_outer = make_hybrid_pavings(space, generic_grids(), create_tree);
        adjoin_outer_approximation(generic_outer, compact_set, 2u);
        Hybrid<Paving> replica_outer = make_hybrid_pavings(space, generic_grids(), create_tree);
        adjoin_outer_approximation(replica_outer, compact_set, 2u);
        mince(generic_outer, 4u);
        mince(replica_outer, 4u);
        ARIADNE_TESTING_EXPECT_EQUAL(generic_outer[first].size(), replica_outer[first].size());
        ARIADNE_TESTING_PROVE(generic_outer[first].subset(static_cast<const PavingInterface&>(replica_outer[first])));
        ARIADNE_TESTING_PROVE(replica_outer[first].subset(static_cast<const PavingInterface&>(generic_outer[first])));
        recombine(generic_outer);
        recombine(replica_outer);
        ARIADNE_TESTING_EXPECT_EQUAL(generic_outer[first].size(), replica_outer[first].size());
        ARIADNE_TESTING_PROVE(not is_empty(generic_outer));

        // Augmented values delegate mince/recombine/is_empty to the slices.
        HybridAutomaton system;
        system.new_mode(first, {dot(x) = -x});
        system.new_mode(second, {dot(y) = -y, dot(x) = -x});
        HybridAugmentedPaving generic_aug = make_hybrid_augmented_paving(generic_grids(), system, create_tree);
        adjoin_outer_approximation(generic_aug, compact_set, 2u);
        ARIADNE_TESTING_PROVE(not is_empty(generic_aug));
        mince(generic_aug, 4u);
        ARIADNE_TESTING_PROVE(generic_aug[first].size() > 0u);
        recombine(generic_aug);
        ARIADNE_TESTING_PROVE(not is_empty(generic_aug));

        // adjoin_inner_approximation on augmented values hits the state paving per location.
        HybridAugmentedPaving generic_aug_inner = make_hybrid_augmented_paving(generic_grids(), system, create_tree);
        adjoin_inner_approximation(generic_aug_inner, wide_set, 2u);
        ARIADNE_TESTING_PROVE(generic_aug_inner.has_value(first));
        ARIADNE_TESTING_PROVE(generic_aug_inner[first].state_paving().size() > 0u);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_hybrid_paving_value_adapters");
    ARIADNE_TESTING_ADD_TEST(TestHybridPavingValueAdapters, test_tree_robdd_and_lazy_locations);
    ARIADNE_TESTING_ADD_TEST(TestHybridPavingValueAdapters, test_extend_auxiliary_preserves_lazy_backend);
    ARIADNE_TESTING_ADD_TEST(TestHybridPavingValueAdapters, test_auxiliary_extension_preserves_named_grid_scalings);
    ARIADNE_TESTING_ADD_TEST(TestHybridPavingValueAdapters, test_set_operations_without_unwanted_materialization);
    ARIADNE_TESTING_ADD_TEST(TestHybridPavingValueAdapters, test_rejects_empty_or_wrong_grid_backend);
    ARIADNE_TESTING_ADD_TEST(TestHybridPavingValueAdapters, test_generic_approximation_and_augmented_value);
    ARIADNE_TESTING_ADD_TEST(TestHybridPavingValueAdapters, test_mince_recombine_is_empty_and_inner_approximation);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

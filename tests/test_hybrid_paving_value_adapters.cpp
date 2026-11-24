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
 *  @brief Compare lazy hybrid paving values with legacy tree and ROBDD storage.
 */

#include <memory>

#include "reasoning/hybrid_paving_value_adapters.hpp"

#ifdef ARIADNE_HYBRID_GRID_HPP
#error Generic paving adapter must not import the legacy HybridGrid header
#endif
#ifdef ARIADNE_HYBRID_SCALINGS_HPP
#error Generic paving adapter must not import legacy HybridScalings
#endif

#include "hybrid/hybrid_paving.hpp"
#include "logging/test_runner.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/robdd_paving.hpp"
#include "reasoning/grid_adapters.hpp"
#include "reasoning/scaling_adapters.hpp"
#include "reasoning/hybrid_paving_value_operations.hpp"
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
            {x.name(), 2.0_x}, {y.name(), 3.0_x}
        };
        const HybridScalings policy(scale_values);
        const HybridGrid legacy_grid(space, policy);

        for (Bool use_robdd : {false, true}) {
            auto manager = std::make_shared<RobddManager>();
            Nat backend_creations = 0u;
            auto create_backend = [manager, use_robdd, &backend_creations](
                                      const Grid& selected_grid) -> Paving {
                ++backend_creations;
                if (use_robdd)
                    return Paving(new RobddPaving(manager, selected_grid));
                return Paving(new BinaryTreePaving(selected_grid));
            };
            HybridPaving legacy(legacy_grid, create_backend);

            Hybrid<Grid> generic_grids = make_hybrid_grids(
                space, make_hybrid_scalings(space, policy));
            Hybrid<Paving> generic = make_hybrid_pavings(
                space, std::move(generic_grids), create_backend);

            ARIADNE_TESTING_PROVE(generic.has_location(first));
            ARIADNE_TESTING_PROVE(generic.has_location(second));
            ARIADNE_TESTING_PROVE(not generic.has_location(absent));
            ARIADNE_TESTING_PROVE(not generic.has_value(first));
            ARIADNE_TESTING_PROVE(not generic.has_value(second));
            ARIADNE_TESTING_EXPECT_EQUAL(generic.size(), 0u);
            ARIADNE_TESTING_EXPECT_EQUAL(backend_creations, 0u);

            const Grid grid_first = legacy_grid[first];
            const GridCell cell_first(grid_first, 0u, BinaryWord());
            Paving& generic_first = generic[first];
            ARIADNE_TESTING_EXPECT_EQUAL(backend_creations, 1u);
            ARIADNE_TESTING_PROVE(generic_first.grid() == grid_first);
            ARIADNE_TESTING_PROVE(not generic.has_value(second));
            generic_first.adjoin(cell_first);
            legacy[first].adjoin(cell_first);
            ARIADNE_TESTING_PROVE(generic_first.superset(cell_first));
            ARIADNE_TESTING_PROVE(legacy[first].superset(cell_first));
            ARIADNE_TESTING_PROVE(generic_first.subset(static_cast<const PavingInterface&>(legacy[first])));
            ARIADNE_TESTING_PROVE(legacy[first].subset(static_cast<const PavingInterface&>(generic_first)));

            const Grid grid_second = legacy_grid[second];
            const GridCell cell_second(grid_second, 0u, BinaryWord());
            Paving& generic_second = generic[second];
            ARIADNE_TESTING_EXPECT_EQUAL(backend_creations, 3u);
            ARIADNE_TESTING_PROVE(generic_second.grid() == grid_second);
            generic_second.adjoin(cell_second);
            legacy[second].adjoin(cell_second);
            ARIADNE_TESTING_PROVE(generic_second.subset(static_cast<const PavingInterface&>(legacy[second])));
            ARIADNE_TESTING_PROVE(legacy[second].subset(static_cast<const PavingInterface&>(generic_second)));
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

    Void test_extend_auxiliary_matches_legacy_and_preserves_lazy_backend() {
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
        const HybridGrid legacy_grid(state_space);
        const Grid grid(1u);
        const GridCell cell(grid, 0u, BinaryWord());
        const auto unit_scalings = [](const DiscreteLocation&, const RealSpace&) {
            return Scalings(1.0_x, Map<RealVariable, ExactDouble>{});
        };

        for (Bool use_robdd : {false, true}) {
            auto manager = std::make_shared<RobddManager>();
            std::function<Paving(const Grid&)> backend =
                [manager, use_robdd](const Grid& selected) -> Paving {
                    if (use_robdd)
                        return Paving(new RobddPaving(manager, selected));
                    return Paving(new BinaryTreePaving(selected));
                };

            HybridPaving legacy(legacy_grid, backend);
            Hybrid<Paving> generic = make_hybrid_pavings(
                state_space, make_hybrid_grids(
                    state_space, Hybrid<Scalings>(state_space, unit_scalings)),
                backend);
            generic[first].adjoin(cell);
            legacy[first].adjoin(cell);
            ARIADNE_TESTING_PROVE(not generic.has_value(second));
            ARIADNE_TESTING_PROVE(not legacy.nontrivial_location(second));

            Hybrid<Paving> expanded = extend_auxiliary(
                generic, Hybrid<Scalings>(expanded_space, unit_scalings),
                backend, automaton);
            HybridPaving legacy_expanded = extend_auxiliary(legacy, automaton);
            ARIADNE_TESTING_PROVE(expanded.has_location(second));
            ARIADNE_TESTING_PROVE(expanded.has_value(first));
            ARIADNE_TESTING_PROVE(not expanded.has_value(second));
            ARIADNE_TESTING_PROVE(not generic.has_value(second));
            ARIADNE_TESTING_EXPECT_EQUAL(expanded.space()[first].dimension(), 2u);

            const Hybrid<Paving>& constant_expanded = expanded;
            const Paving& actual = constant_expanded[first];
            const Paving& expected = legacy_expanded[first];
            ARIADNE_TESTING_PROVE(actual.grid() == expected.grid());
            for (const auto& selected_cell : actual)
                ARIADNE_TESTING_PROVE(expected.superset(selected_cell));
            for (const auto& selected_cell : expected)
                ARIADNE_TESTING_PROVE(actual.superset(selected_cell));
            ARIADNE_TESTING_PROVE(not actual.is_empty());

            const Hybrid<Paving> copied(expanded);
            ARIADNE_TESTING_PROVE(not copied.has_value(second));
            const Paving& snapshot = copied[first];
            for (const auto& selected_cell : expected)
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
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_hybrid_paving_value_adapters");
    ARIADNE_TESTING_ADD_TEST(TestHybridPavingValueAdapters, test_tree_robdd_and_lazy_locations);
    ARIADNE_TESTING_ADD_TEST(TestHybridPavingValueAdapters, test_extend_auxiliary_matches_legacy_and_preserves_lazy_backend);
    ARIADNE_TESTING_ADD_TEST(TestHybridPavingValueAdapters, test_rejects_empty_or_wrong_grid_backend);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

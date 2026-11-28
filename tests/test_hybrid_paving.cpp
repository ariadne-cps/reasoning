/***************************************************************************
 *            tests/test_hybrid_paving.cpp
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

/** @file tests/test_hybrid_paving.cpp
 *  @brief Generic Hybrid<Paving> auxiliary extension: lazy locations and backend preservation.
 */

#include "hybrid/hybrid_set.hpp"
#include "logging/test_runner.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/robdd_paving.hpp"
#include "reasoning/hybrid_grid_value.hpp"
#include "reasoning/hybrid_paving_value.hpp"
#include "symbolic/assignment.hpp"
#include "system/hybrid_automaton.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestHybridPaving : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_extend_auxiliary() {
        RealVariable x("x"), a("a");
        StringVariable q("q");
        DiscreteLocation loc({q | "1"});

        // A mode with an auxiliary (algebraic) variable a = x + 1.
        HybridAutomaton automaton;
        ARIADNE_TESTING_EXPECT_NO_THROW(automaton.new_mode(loc,
                                                           List<RealAssignment>{let(a) = x + 1},
                                                           List<DottedRealAssignment>{dot(x) = -x}));

        const HybridSpace state_space = automaton.state_space();
        const HybridSpace expanded_space = automaton.state_auxiliary_space();
        const auto unit_scalings = [](const DiscreteLocation&, const RealSpace&) {
            return Scalings(1.0_x, Map<RealVariable, ExactDouble>{});
        };

        // A one-cell state paving in location loc.
        HybridExactBoxes boxes;
        boxes.insert(loc, RealSpace({
                              x
        }),
                     ExactBoxType({{0, 1}}));

        for (Bool use_robdd: {false, true}) {
            auto manager = std::make_shared<RobddManager>();
            std::function<Paving(const Grid&)> backend =
                [manager, use_robdd](const Grid& selected) -> Paving {
                if (use_robdd)
                    return Paving(new RobddPaving(manager, selected));
                return Paving(new BinaryTreePaving(selected));
            };

            Hybrid<Paving> paving = make_hybrid_pavings(
                state_space, make_hybrid_grids(state_space, Hybrid<Scalings>(state_space, unit_scalings)),
                backend);
            adjoin_outer_approximation(paving, boxes, 3u);
            ARIADNE_TESTING_PROVE(paving.has_value(loc));
            ARIADNE_TESTING_PROVE(paving[loc].size() > 0u);

            // Extending with the auxiliary variable must produce a paving over the
            // combined state+auxiliary space, with the same location and cells.
            Hybrid<Paving> extended = extend_auxiliary(
                paving, Hybrid<Scalings>(expanded_space, unit_scalings), backend, automaton);
            ARIADNE_TESTING_PROVE(extended.has_value(loc));
            ARIADNE_TESTING_PROVE(extended[loc].size() > 0u);
            ARIADNE_TESTING_PROVE(extended.space().has_location(loc));
            ARIADNE_TESTING_EXPECT_EQUAL(extended.space()[loc].dimension(), 2u);
            ARIADNE_TESTING_NOTE(extended);

            // Auxiliary extension must retain the selected backend representation.
            const PavingInterface& slice = static_cast<const PavingInterface&>(extended[loc]);
            if (use_robdd)
                ARIADNE_TESTING_PROVE(dynamic_cast<const RobddPaving*>(&slice) != nullptr);
            else
                ARIADNE_TESTING_PROVE(dynamic_cast<const BinaryTreePaving*>(&slice) != nullptr);
        }

        // Compare geometry rather than storage-specific leaf counts between the
        // tree and ROBDD backends.
        auto manager = std::make_shared<RobddManager>();
        std::function<Paving(const Grid&)> tree_backend =
            [](const Grid& selected) { return Paving(new BinaryTreePaving(selected)); };
        std::function<Paving(const Grid&)> robdd_backend =
            [manager](const Grid& selected) { return Paving(new RobddPaving(manager, selected)); };
        Hybrid<Paving> tree = make_hybrid_pavings(
            state_space, make_hybrid_grids(state_space, Hybrid<Scalings>(state_space, unit_scalings)),
            tree_backend);
        Hybrid<Paving> robdd = make_hybrid_pavings(
            state_space, make_hybrid_grids(state_space, Hybrid<Scalings>(state_space, unit_scalings)),
            robdd_backend);
        adjoin_outer_approximation(tree, boxes, 3u);
        adjoin_outer_approximation(robdd, boxes, 3u);
        Hybrid<Paving> extended_tree = extend_auxiliary(
            tree, Hybrid<Scalings>(expanded_space, unit_scalings), tree_backend, automaton);
        Hybrid<Paving> extended_robdd = extend_auxiliary(
            robdd, Hybrid<Scalings>(expanded_space, unit_scalings), robdd_backend, automaton);
        const Paving& tree_slice = extended_tree[loc];
        const Paving& robdd_slice = extended_robdd[loc];
        for (const auto& cell: tree_slice)
            ARIADNE_TESTING_PROVE(robdd_slice.superset(cell));
        for (const auto& cell: robdd_slice)
            ARIADNE_TESTING_PROVE(tree_slice.superset(cell));
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_hybrid_paving");
    ARIADNE_TESTING_ADD_TEST(TestHybridPaving, test_extend_auxiliary);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

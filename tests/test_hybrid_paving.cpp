/***************************************************************************
 *            test_hybrid_paving.cpp
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne Reasoning.  If not, see <https://www.gnu.org/licenses/>.
 */

/** @file tests/test_hybrid_paving.cpp
 *  @brief Test hybrid paving behavior.
 */

#include <memory>

#include "hybrid/hybrid_bounded_constraint_set.hpp"
#include "hybrid/hybrid_paving.hpp"
#include "hybrid/hybrid_space.hpp"
#include "logging/test_runner.hpp"
#include "paving/robdd_paving.hpp"
#include "system/hybrid_automaton.hpp"
#include "reasoning/hybrid_paving.hpp"

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

        // A one-cell state paving in location loc.
        RealSpace space({x});
        MonolithicHybridSpace hspace;
        hspace.new_location(loc, space);
        HybridGrid grid(hspace);
        HybridPaving paving(grid);
        HybridExactBoxes boxes;
        boxes.insert(loc, space, ExactBoxType({
                                     {0, 1}
        }));
        paving.adjoin_outer_approximation(boxes, 3u);
        ARIADNE_TESTING_PROVE(paving.nontrivial_location(loc));

        // Extending with the auxiliary variable must produce a paving over the
        // combined state+auxiliary space, with the same location and cells.
        HybridPaving extended = extend_auxiliary(paving, automaton);
        ARIADNE_TESTING_PROVE(extended.nontrivial_location(loc));
        ARIADNE_TESTING_PROVE(extended.size() > 0u);
        ARIADNE_TESTING_PROVE(extended.grid().has_location(loc));
        ARIADNE_TESTING_EXPECT_EQUAL(extended.grid().space()[loc].size(), 2u);
        ARIADNE_TESTING_NOTE(extended);

        // Auxiliary extension must retain the selected ROBDD representation.
        auto manager = std::make_shared<RobddManager>();
        HybridPaving robdd(grid, manager);
        robdd.adjoin_outer_approximation(boxes, 3u);
        HybridPaving extended_robdd = extend_auxiliary(robdd, automaton);
        ARIADNE_TESTING_PROVE(extended_robdd.size() > 0u);
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const RobddPaving *>(
                &static_cast<const PavingInterface &>(extended_robdd[loc])) != nullptr);
        // Compare geometry rather than storage-specific leaf counts. The two
        // implementations need not use matching subdivisions or BDD managers.
        const Paving &tree_slice = extended[loc];
        const Paving &robdd_slice = extended_robdd[loc];
        for (const auto &cell: tree_slice)
            ARIADNE_TESTING_PROVE(robdd_slice.superset(cell));
        for (const auto &cell: robdd_slice)
            ARIADNE_TESTING_PROVE(tree_slice.superset(cell));
    }
};

int main(int argc, char **argv) {
    ARIADNE_TESTING_SUITE("test_hybrid_paving");
    ARIADNE_TESTING_ADD_TEST(TestHybridPaving, test_extend_auxiliary);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

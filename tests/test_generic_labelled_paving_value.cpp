/***************************************************************************
 *            tests/test_generic_labelled_paving_value.cpp
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

/** @file tests/test_generic_labelled_paving_value.cpp
 *  @brief Test generic labelled augmented pavings independently of legacy wrappers.
 */

#include <memory>

#include "reasoning/labelled_paving_value.hpp"

#ifdef ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_HPP
#error Generic labelled paving values must not import the legacy augmented paving type
#endif

#include "logging/test_runner.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/robdd_paving.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestGenericLabelledPavingValue : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_set_operations_ignore_auxiliary_labels_but_preserve_ownership() {
        const RealVariable x("x"), y("y"), a("a"), b("b");
        const RealSpace state({y, x}), auxiliary_a({a}), auxiliary_b({b});
        const Grid grid(2u);
        const GridCell cell(grid, 0u, BinaryWord());
        const Mapping mapping({EffectiveScalarMultivariateFunction::coordinate(2u, 0u)});

        BinaryTreePaving tree(grid);
        tree.adjoin(cell);
        const Paving occupied(new BinaryTreePaving(tree));
        const Paving empty(new BinaryTreePaving(grid));

        auto left = make_labelled_augmented_paving(
            AugmentedPaving(empty, mapping), state, auxiliary_a);
        const auto right = make_labelled_augmented_paving(
            AugmentedPaving(occupied, mapping), state, auxiliary_b);

        // The old set algebra compares ordered state labels only. The two
        // auxiliary label spaces are deliberately different.
        ARIADNE_TESTING_PROVE(subset(left, right));
        ARIADNE_TESTING_PROVE(not intersect(left, right));
        adjoin(left, right);
        ARIADNE_TESTING_PROVE(left.value().state_paving().superset(cell));
        ARIADNE_TESTING_PROVE(intersect(left, right));
        ARIADNE_TESTING_EXPECT_EQUAL(auxiliary_space(left), auxiliary_a);
        ARIADNE_TESTING_EXPECT_EQUAL(state_space(left), state);

        restrict(left, right);
        ARIADNE_TESTING_PROVE(left.value().state_paving().superset(cell));
        remove(left, right);
        ARIADNE_TESTING_PROVE(left.value().is_empty());
        ARIADNE_TESTING_EXPECT_EQUAL(auxiliary_space(left), auxiliary_a);

        const auto wrong_order = make_labelled_augmented_paving(
            AugmentedPaving(occupied, mapping), RealSpace({x, y}), auxiliary_a);
        ARIADNE_TESTING_EXPECT_EXCEPTION(adjoin(left, wrong_order));
        ARIADNE_TESTING_EXPECT_EXCEPTION(restrict(left, wrong_order));
        ARIADNE_TESTING_EXPECT_EXCEPTION(remove(left, wrong_order));
        ARIADNE_TESTING_EXPECT_EXCEPTION(subset(left, wrong_order));
        ARIADNE_TESTING_EXPECT_EXCEPTION(intersect(left, wrong_order));
    }

    Void test_robdd_copy_and_factory_dimension_checks() {
        const RealVariable x("x"), y("y"), a("a"), b("b");
        const RealSpace state({y, x}), auxiliary({a});
        const Grid grid(2u);
        const GridCell cell(grid, 0u, BinaryWord());
        const Mapping mapping({EffectiveScalarMultivariateFunction::coordinate(2u, 0u)});

        const auto manager = std::make_shared<RobddManager>();
        RobddPaving robdd(manager, grid);
        robdd.adjoin(cell);
        const Paving original(new RobddPaving(robdd));
        const auto labelled = make_labelled_augmented_paving(
            AugmentedPaving(original, mapping), state, auxiliary);
        const LabelledAugmentedPaving copy(labelled);
        ARIADNE_TESTING_PROVE(copy.value().state_paving().superset(cell));
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const RobddPaving*>(
                &static_cast<const PavingInterface&>(copy.value().state_paving())) != nullptr);
        ARIADNE_TESTING_EXPECT_EQUAL(state_auxiliary_space(copy),
                                     RealSpace({y, x, a}));

        ARIADNE_TESTING_EXPECT_EXCEPTION(make_labelled_augmented_paving(
            AugmentedPaving(original, mapping), RealSpace({x}), auxiliary));
        ARIADNE_TESTING_EXPECT_EXCEPTION(make_labelled_augmented_paving(
            AugmentedPaving(original, mapping), state, RealSpace({a, b})));
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_generic_labelled_paving_value");
    ARIADNE_TESTING_ADD_TEST(TestGenericLabelledPavingValue,
                             test_set_operations_ignore_auxiliary_labels_but_preserve_ownership);
    ARIADNE_TESTING_ADD_TEST(TestGenericLabelledPavingValue,
                             test_robdd_copy_and_factory_dimension_checks);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

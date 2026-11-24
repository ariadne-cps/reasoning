/***************************************************************************
 *            tests/test_labelled_augmented_paving_adapters.cpp
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

/** @file tests/test_labelled_augmented_paving_adapters.cpp
 *  @brief Compare generic labelled augmented pavings to the legacy implementation.
 */

#include <memory>

#include "labelled/labelled_augmented_paving.hpp"
#include "logging/test_runner.hpp"
#include "paving/robdd_paving.hpp"
#include "reasoning/labelled_paving_value.hpp"
#include "reasoning/labelled_augmented_paving_value_drawing.hpp"
#include "reasoning/labelled_paving_drawing.hpp"
#include "reasoning/paving_drawing.hpp"
#include "rendering/null_rendering.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class PavingBoundaryCanvas : public NullCanvas {
  public:
    Nat boundaries = 0u;
    Void fill_boundary(const List<Point2d>&) override { ++boundaries; }
};

class TestLabelledAugmentedPavingAdapters : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_metadata_geometry_backends_and_drawing() {
        const RealVariable x("x"), y("y"), a("a");
        const RealSpace state({y, x}), auxiliary({a});
        const Grid grid(2u);
        const GridCell cell(grid, 0u, BinaryWord());
        const Mapping mapping({EffectiveScalarMultivariateFunction::coordinate(2u, 0u)});

        BinaryTreePaving tree(grid);
        tree.adjoin(cell);
        const Paving tree_storage(new BinaryTreePaving(tree));
        const LabelledAugmentedPaving legacy_tree(tree_storage, state, mapping, auxiliary);
        const auto generic_tree = make_labelled_augmented_paving(
            AugmentedPaving(tree_storage, mapping), state, auxiliary);
        ARIADNE_TESTING_EXPECT_EQUAL(state_space(generic_tree), legacy_tree.state_space());
        ARIADNE_TESTING_EXPECT_EQUAL(auxiliary_space(generic_tree), legacy_tree.auxiliary_space());
        ARIADNE_TESTING_EXPECT_EQUAL(state_auxiliary_space(generic_tree),
                                     legacy_tree.state_auxiliary_space());
        ARIADNE_TESTING_EXPECT_EQUAL(generic_tree.value().auxiliary_mapping().result_size(), 1u);
        ARIADNE_TESTING_PROVE(generic_tree.value().state_paving().superset(cell));
        ARIADNE_TESTING_PROVE(legacy_tree.state_paving().superset(cell));
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const BinaryTreePaving*>(
                &static_cast<const PavingInterface&>(generic_tree.value().state_paving())) != nullptr);

        const auto manager = std::make_shared<RobddManager>();
        RobddPaving robdd(manager, grid);
        robdd.adjoin(cell);
        const Paving robdd_storage(new RobddPaving(robdd));
        const LabelledAugmentedPaving legacy_robdd(robdd_storage, state, mapping, auxiliary);
        const auto generic_robdd = make_labelled_augmented_paving(
            AugmentedPaving(robdd_storage, mapping), state, auxiliary);
        ARIADNE_TESTING_EXPECT_EQUAL(state_space(generic_robdd), legacy_robdd.state_space());
        ARIADNE_TESTING_PROVE(generic_robdd.value().state_paving().superset(cell));
        ARIADNE_TESTING_PROVE(legacy_robdd.state_paving().superset(cell));

        const LabelledAugmentedPavingValue copy(generic_robdd);
        ARIADNE_TESTING_PROVE(copy.value().state_paving().superset(cell));
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const RobddPaving*>(
                &static_cast<const PavingInterface&>(copy.value().state_paving())) != nullptr);

        // Rendering materializes the auxiliary graph, not a replacement for
        // the authoritative ROBDD-backed state paving.
        NullCanvas canvas;
        const Projection2d projection(3u, 0u, 2u);
        PavingDrawing::draw(canvas, projection, copy.value());
        ARIADNE_TESTING_PROVE(copy.value().state_paving().superset(cell));

        // Select one state and one auxiliary variable. The generic adapter
        // must use the same ordered projection and draw the same cell graph.
        PavingBoundaryCanvas legacy_canvas, generic_canvas;
        PavingDrawing::draw(legacy_canvas, Variables2d(y, a), legacy_robdd);
        PavingDrawing::draw(generic_canvas, Variables2d(y, a), copy);
        ARIADNE_TESTING_PROVE(legacy_canvas.boundaries > 0u);
        ARIADNE_TESTING_EXPECT_EQUAL(generic_canvas.boundaries,
                                     legacy_canvas.boundaries);

        PavingDrawing::LabelledAugmentedPavingValueDrawable drawable(copy);
        std::unique_ptr<LabelledDrawable2dInterface> cloned(drawable.labelled_clone());
        PavingBoundaryCanvas clone_canvas;
        cloned->draw(clone_canvas, Variables2d(y, a));
        ARIADNE_TESTING_EXPECT_EQUAL(clone_canvas.boundaries,
                                     legacy_canvas.boundaries);
        LabelledFigure figure(Axes2d(-2.0, y, 2.0, -2.0, a, 2.0));
        PavingDrawing::draw(figure, copy);
        PavingBoundaryCanvas figure_canvas;
        figure._paint_all(figure_canvas);
        ARIADNE_TESTING_EXPECT_EQUAL(figure_canvas.boundaries,
                                     legacy_canvas.boundaries);
        ARIADNE_TESTING_PROVE(copy.value().state_paving().superset(cell));

        ARIADNE_TESTING_EXPECT_EXCEPTION(make_labelled_augmented_paving(
            AugmentedPaving(tree_storage, mapping), RealSpace({x}), auxiliary));
        ARIADNE_TESTING_EXPECT_EXCEPTION(make_labelled_augmented_paving(
            AugmentedPaving(tree_storage, mapping), state, RealSpace({a, x})));
    }

    Void test_labelled_set_operations_match_legacy() {
        const RealVariable x("x"), y("y"), a("a");
        const RealSpace state({y, x}), other_state({x, y}), auxiliary({a});
        const Grid grid(2u);
        const GridCell cell(grid, 0u, BinaryWord());
        const Mapping mapping({EffectiveScalarMultivariateFunction::coordinate(2u, 0u)});

        BinaryTreePaving tree(grid);
        tree.adjoin(cell);
        const Paving occupied(new BinaryTreePaving(tree));
        const Paving empty(new BinaryTreePaving(grid));

        LabelledAugmentedPaving legacy_empty(empty, state, mapping, auxiliary);
        const LabelledAugmentedPaving legacy_full(occupied, state, mapping, auxiliary);
        auto generic_empty = make_labelled_augmented_paving(
            AugmentedPaving(empty, mapping), state, auxiliary);
        const auto generic_full = make_labelled_augmented_paving(
            AugmentedPaving(occupied, mapping), state, auxiliary);
        ARIADNE_TESTING_PROVE(subset(generic_empty, generic_full));
        ARIADNE_TESTING_PROVE(subset(legacy_empty, legacy_full));
        ARIADNE_TESTING_PROVE(not intersect(generic_empty, generic_full));
        ARIADNE_TESTING_PROVE(not intersect(legacy_empty, legacy_full));

        adjoin(generic_empty, generic_full);
        legacy_empty.adjoin(legacy_full);
        ARIADNE_TESTING_PROVE(generic_empty.value().state_paving().superset(cell));
        ARIADNE_TESTING_PROVE(legacy_empty.state_paving().superset(cell));
        ARIADNE_TESTING_PROVE(intersect(generic_empty, generic_full));
        ARIADNE_TESTING_PROVE(intersect(legacy_empty, legacy_full));
        restrict(generic_empty, generic_full);
        legacy_empty.restrict(legacy_full);
        ARIADNE_TESTING_PROVE(generic_empty.value().state_paving().superset(cell));
        remove(generic_empty, generic_full);
        legacy_empty.remove(legacy_full);
        ARIADNE_TESTING_PROVE(generic_empty.value().is_empty());
        ARIADNE_TESTING_PROVE(legacy_empty.is_empty());

        const auto different = make_labelled_augmented_paving(
            AugmentedPaving(occupied, mapping), other_state, auxiliary);
        ARIADNE_TESTING_EXPECT_EXCEPTION(adjoin(generic_empty, different));
        ARIADNE_TESTING_EXPECT_EXCEPTION(restrict(generic_empty, different));
        ARIADNE_TESTING_EXPECT_EXCEPTION(remove(generic_empty, different));
        ARIADNE_TESTING_EXPECT_EXCEPTION(subset(generic_empty, different));
        ARIADNE_TESTING_EXPECT_EXCEPTION(intersect(generic_empty, different));
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_labelled_augmented_paving_adapters");
    ARIADNE_TESTING_ADD_TEST(TestLabelledAugmentedPavingAdapters, test_metadata_geometry_backends_and_drawing);
    ARIADNE_TESTING_ADD_TEST(TestLabelledAugmentedPavingAdapters, test_labelled_set_operations_match_legacy);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

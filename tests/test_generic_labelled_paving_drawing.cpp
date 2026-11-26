/***************************************************************************
 *            tests/test_generic_labelled_paving_drawing.cpp
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

/** @file tests/test_generic_labelled_paving_drawing.cpp
 *  @brief Exercise generic labelled paving drawing without importing its legacy counterpart.
 */

#include <memory>

#include "reasoning/labelled_augmented_paving_value_drawing.hpp"

#ifdef ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_HPP
#error The generic drawing header must not import labelled/labelled_augmented_paving.hpp
#endif

#include "logging/test_runner.hpp"
#include "paving/robdd_paving.hpp"
#include "rendering/null_rendering.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class CountingPavingCanvas final : public NullCanvas {
  public:
    Nat boundaries = 0u;
    Void fill_boundary(const List<Point2d>&) override { ++boundaries; }
};

class TestGenericLabelledPavingDrawing : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_tree_robdd_and_owned_figures() {
        const RealVariable x("x"), y("y"), a("a");
        const RealSpace state({y, x}), auxiliary({a});
        const Grid grid(2u);
        const GridCell cell(grid, 0u, BinaryWord());
        const Mapping mapping({EffectiveScalarMultivariateFunction::coordinate(2u, 0u)});

        BinaryTreePaving tree(grid);
        tree.adjoin(cell);
        const Paving tree_state(new BinaryTreePaving(tree));
        const auto labelled_tree = make_labelled_augmented_paving(
            AugmentedPaving(tree_state, mapping), state, auxiliary);
        CountingPavingCanvas tree_canvas;
        PavingDrawing::draw(tree_canvas, Variables2d(y, a), labelled_tree);
        ARIADNE_TESTING_PROVE(tree_canvas.boundaries > 0u);

        const auto manager = std::make_shared<RobddManager>();
        RobddPaving robdd(manager, grid);
        robdd.adjoin(cell);
        const Paving robdd_state(new RobddPaving(robdd));
        const auto labelled_robdd = make_labelled_augmented_paving(
            AugmentedPaving(robdd_state, mapping), state, auxiliary);
        CountingPavingCanvas robdd_canvas;
        PavingDrawing::draw(robdd_canvas, Variables2d(y, a), labelled_robdd);
        ARIADNE_TESTING_EXPECT_EQUAL(robdd_canvas.boundaries, tree_canvas.boundaries);

        // Both figure interfaces must own drawable snapshots. The ROBDD
        // continues to be the state backend after the auxiliary graph is drawn.
        PavingDrawing::LabelledAugmentedPavingValueDrawable drawable(labelled_robdd);
        std::unique_ptr<LabelledDrawable2dInterface> clone(drawable.labelled_clone());
        CountingPavingCanvas clone_canvas;
        clone->draw(clone_canvas, Variables2d(y, a));
        ARIADNE_TESTING_EXPECT_EQUAL(clone_canvas.boundaries, tree_canvas.boundaries);

        LabelledFigure labelled_figure(Axes2d(-2.0, y, 2.0, -2.0, a, 2.0));
        PavingDrawing::draw(labelled_figure, labelled_robdd);
        CountingPavingCanvas figure_canvas;
        labelled_figure._paint_all(figure_canvas);
        ARIADNE_TESTING_EXPECT_EQUAL(figure_canvas.boundaries, tree_canvas.boundaries);

        Figure figure(ApproximateBoxType({
                          {-2, 2},
                          {-2, 2},
                          {-2, 2}
        }),
                      Projection2d(3u, 0u, 2u));
        PavingDrawing::draw(figure, labelled_robdd);
        CountingPavingCanvas euclidean_canvas;
        figure._paint_all(euclidean_canvas);
        ARIADNE_TESTING_EXPECT_EQUAL(euclidean_canvas.boundaries, tree_canvas.boundaries);
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const RobddPaving*>(
                &static_cast<const PavingInterface&>(labelled_robdd.value().state_paving())) != nullptr);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_generic_labelled_paving_drawing");
    ARIADNE_TESTING_ADD_TEST(TestGenericLabelledPavingDrawing, test_tree_robdd_and_owned_figures);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

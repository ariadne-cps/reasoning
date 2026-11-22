/***************************************************************************
 *            tests/test_paving_drawing.cpp
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

/** @file tests/test_paving_drawing.cpp
 *  @brief Paving renderers migrated from Drawing to Reasoning.
 */

#include <memory>
#include <type_traits>

#include "logging/test_runner.hpp"
#include "paving/robdd_paving.hpp"
#include "reasoning/paving_drawing.hpp"
#include "rendering/null_rendering.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class PavingDrawingCanvas final : public NullCanvas {
  public:
    Nat boundaries = 0u;
    Void fill_boundary(const List<Point2d>&) override { ++boundaries; }
};

class TestPavingDrawing : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_generic_paving_and_figure_ownership() {
        static_assert(not std::is_base_of_v<Drawable2dInterface, SubPavingInterface>);
        static_assert(not std::is_base_of_v<Drawable2dInterface, BinaryTreePaving>);

        Grid grid(2u);
        BinaryTreePaving tree(grid);
        tree.adjoin(GridCell(grid, 0u, BinaryWord()));

        PavingDrawingCanvas canvas;
        Projection2d projection(2u, 0u, 1u);
        PavingDrawing::draw(canvas, projection, static_cast<const SubPavingInterface&>(tree));
        ARIADNE_TESTING_EXPECT_EQUAL(canvas.boundaries, 1u);

        Figure figure(ApproximateBoxType({
                          {-2, +2},
                          {-2, +2}
        }), projection);
        Paving handle(new BinaryTreePaving(tree));
        SubPaving borrowed(static_cast<const SubPavingInterface&>(tree));
        PavingDrawing::draw(figure, static_cast<const SubPavingInterface&>(tree));
        PavingDrawing::draw(figure, handle);
        PavingDrawing::draw(figure, borrowed);
        PavingDrawing::draw(figure, static_cast<const SubPavingInterface&>(tree));
        PavingDrawing::draw(figure, static_cast<const SubPavingInterface&>(tree));
        PavingDrawing::draw(figure, handle);
        PavingDrawing::draw(figure, borrowed);

        // Figure has taken clones, not borrowed references to the original.
        figure._paint_all(canvas);
        ARIADNE_TESTING_EXPECT_EQUAL(canvas.boundaries, 8u);
    }

    Void test_cells_and_borrowed_binary_tree_views() {
        GridCell cell(Grid(1u), 0u, BinaryWord("01"));
        BinaryTreePaving set(cell);
        auto product_set = product(cell, set);
        PavingDrawingCanvas canvas;
        Projection2d projection(2u, 0u, 1u);

        PavingDrawing::draw(canvas, projection, *product_set.begin());
        ARIADNE_TESTING_EXPECT_EQUAL(canvas.boundaries, 1u);
        PavingDrawing::draw(canvas, projection,
                            static_cast<const SubPavingInterface&>(product_set));
        ARIADNE_TESTING_EXPECT_EQUAL(canvas.boundaries, 2u);

        const BinaryTreeSubpaving& subpaving = product_set;
        PavingDrawing::draw(canvas, projection, subpaving);
        ARIADNE_TESTING_EXPECT_EQUAL(canvas.boundaries, 3u);
        BinaryTreePaving empty_paving(2u);
        PavingDrawing::draw(canvas, projection,
                            static_cast<const BinaryTreeSubpaving&>(empty_paving));
        ARIADNE_TESTING_EXPECT_EQUAL(canvas.boundaries, 3u);

        Figure figure(ApproximateBoxType({
                          {-2, +2},
                          {-2, +2}
        }), projection);
        PavingDrawing::draw(figure, *product_set.begin());
        PavingDrawing::draw(figure, subpaving);
        PavingDrawing::draw(figure, *product_set.begin());
        PavingDrawing::draw(figure, *product_set.begin());
        PavingDrawingCanvas figure_canvas;
        figure._paint_all(figure_canvas);
        ARIADNE_TESTING_EXPECT_EQUAL(figure_canvas.boundaries, 4u);
    }

    Void test_augmented_paving_graph_and_cloning() {
        static_assert(not std::is_base_of_v<Drawable2dInterface, AugmentedPaving>);
        Grid grid(2u);
        EffectiveVectorMultivariateFunction f =
            EffectiveVectorMultivariateFunction::identity(2u);
        AugmentedPaving paving(grid, f);
        paving.adjoin_outer_approximation(ExactBoxType({
                                              {-1, 1},
                                              {-1, 1}
                                          }), 2u);
        Projection2d projection(4u, 0u, 1u);
        PavingDrawingCanvas canvas;
        PavingDrawing::draw(canvas, projection, paving);
        ARIADNE_TESTING_PROVE(canvas.boundaries > 0u);

        PavingDrawing::AugmentedPavingDrawable drawable(paving);
        ARIADNE_TESTING_EXPECT_EQUAL(paving.dimension(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(drawable.dimension(), 4u);
        ARIADNE_TESTING_EXPECT_EQUAL(drawable.dimension(),
                                     paving.state_auxiliary_paving().dimension());
        std::unique_ptr<Drawable2dInterface> copy(drawable.clone());
        ARIADNE_TESTING_EXPECT_EQUAL(copy->dimension(), drawable.dimension());
        copy->draw(canvas, projection);
        ARIADNE_TESTING_PROVE(canvas.boundaries > 0u);

        Figure figure(ApproximateBoxType({
                          {-2, +2},
                          {-2, +2},
                          {-2, +2},
                          {-2, +2}
        }), projection);
        PavingDrawing::draw(figure, paving);
        PavingDrawing::draw(figure, paving);
        const Nat previous = canvas.boundaries;
        figure._paint_all(canvas);
        ARIADNE_TESTING_PROVE(canvas.boundaries > previous);
    }

    Void test_robdd_storage_drawing() {
        Grid grid(2u);
        auto manager = std::make_shared<RobddManager>();
        RobddPaving paving(manager, grid);
        paving.adjoin(GridCell(grid, 0u, BinaryWord()));
        PavingDrawingCanvas canvas;
        PavingDrawing::draw(canvas, Projection2d(2u, 0u, 1u),
                            static_cast<const SubPavingInterface&>(paving));
        ARIADNE_TESTING_EXPECT_EQUAL(canvas.boundaries, 1u);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_paving_drawing");
    ARIADNE_TESTING_ADD_TEST(TestPavingDrawing, test_generic_paving_and_figure_ownership);
    ARIADNE_TESTING_ADD_TEST(TestPavingDrawing, test_cells_and_borrowed_binary_tree_views);
    ARIADNE_TESTING_ADD_TEST(TestPavingDrawing, test_augmented_paving_graph_and_cloning);
    ARIADNE_TESTING_ADD_TEST(TestPavingDrawing, test_robdd_storage_drawing);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

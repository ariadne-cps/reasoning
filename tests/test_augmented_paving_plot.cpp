/***************************************************************************
 *            tests/test_augmented_paving_plot.cpp
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

/** @file tests/test_augmented_paving_plot.cpp
 *  @brief Validate backend-neutral coloured paving plotting and drawing order.
 */

#include "reasoning/augmented_paving_plot.hpp"

#ifdef ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_HPP
#error Coloured augmented paving plots must not import the legacy labelled paving
#endif

#include <memory>
#include <vector>

#include "logging/test_runner.hpp"
#include "paving/robdd_paving.hpp"
#include "rendering/null_rendering.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class ColouredPavingCanvas final : public NullCanvas {
  public:
    double red = -1.0;
    std::vector<double> rendered_colours;

    Void set_fill_colour(double r, double, double) override { red = r; }
    Void fill_boundary(const List<Point2d>&) override {
        rendered_colours.push_back(red);
    }
};

class TestAugmentedPavingPlot : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_colours_order_and_backend() {
        const Grid grid(2u);
        const GridCell cell(grid, 0u, BinaryWord());
        const Mapping mapping({EffectiveScalarMultivariateFunction::coordinate(2u, 0u)});

        BinaryTreePaving tree(grid);
        tree.adjoin(cell);
        const Paving tree_storage(new BinaryTreePaving(tree));
        AugmentedPaving tree_value(tree_storage, mapping);

        const auto manager = std::make_shared<RobddManager>();
        RobddPaving robdd(manager, grid);
        robdd.adjoin(cell);
        const Paving robdd_storage(new RobddPaving(robdd));
        AugmentedPaving robdd_value(robdd_storage, mapping);

        Figure plotted(ApproximateBoxType({{-2, 2}, {-2, 2}, {-2, 2}}),
                       Projection2d(3u, 0u, 2u));
        PavingDrawing::draw_coloured_pavings(plotted,
                                             Colour(0.25, 0.0, 0.0), tree_value,
                                             Colour(0.75, 0.0, 0.0), robdd_value);
        ColouredPavingCanvas rendered;
        plotted._paint_all(rendered);
        ARIADNE_TESTING_PROVE(not rendered.rendered_colours.empty());

        // Each drawing layer must retain its chosen colour, and the layers
        // must be rendered in insertion order regardless of their backend.
        bool seen_tree = false;
        bool seen_robdd = false;
        for (double red_component : rendered.rendered_colours) {
            if (red_component == 0.25) {
                ARIADNE_TESTING_PROVE(not seen_robdd);
                seen_tree = true;
            } else if (red_component == 0.75) {
                ARIADNE_TESTING_PROVE(seen_tree);
                seen_robdd = true;
            } else {
                ARIADNE_TESTING_PROVE(false);
            }
        }
        ARIADNE_TESTING_PROVE(seen_tree);
        ARIADNE_TESTING_PROVE(seen_robdd);
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const RobddPaving*>(
                &static_cast<const PavingInterface&>(robdd_value.state_paving())) != nullptr);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_augmented_paving_plot");
    ARIADNE_TESTING_ADD_TEST(TestAugmentedPavingPlot, test_colours_order_and_backend);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

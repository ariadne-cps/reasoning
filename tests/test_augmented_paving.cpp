/***************************************************************************
 *            test_augmented_paving.cpp
 *
 *  Copyright  2006-20  Luca Geretti
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

/** @file tests/test_augmented_paving.cpp
 *  @brief Test augmented paving semantics and regressions.
 */

#include <iostream>
#include <memory>

#include "algebra/vector.hpp"
#include "drawing/drawer.hpp"
#include "drawing/drawer_manager.hpp"
#include "drawing/labelled_figure.hpp"
#include "evolution/enclosure.hpp"
#include "evolution/orbit.hpp"
#include "evolution/vector_field_evolver.hpp"
#include "function/function.hpp"
#include "geometry/box.hpp"
#include "integration/integrator.hpp"
#include "labelled/labelled_mapping.hpp"
#include "logging/test_runner.hpp"
#include "numeric/numeric.hpp"
#include "paving/augmented_paving.hpp"
#include "paving/binary_tree_paving.hpp"
#include "reasoning/augmented_paving_orbit.hpp"
#include "reasoning/enclosure_augmented_paving.hpp"
#include "reasoning/labelled_augmented_paving_value_drawing.hpp"
#include "reasoning/labelled_paving_value.hpp"
#include "rendering/null_rendering.hpp"
#include "set/function_set.hpp"
#include "set/list_set.hpp"
#include "system/vector_field.hpp"
#include "utility/tuple.hpp"

#include "ariadne-testing.hpp"
#include "expression-set/expression_set.hpp"

using namespace Ariadne;
using namespace std;

class TestAugmentedPaving : public Ariadne::Testing::ContextFixture {
    RealVariable x;
    RealVariable y;
    RealVariable a;
    VectorField system;
    Grid grid;

  public:
    using ContextFixture::ContextFixture;

    TestAugmentedPaving(Ariadne::Testing::Context& context)
        : ContextFixture(context),
          x("x"), y("y"), a("a"),
          system(List<DottedRealAssignment>{dot(x) = -x, dot(y) = -y},
                 List<RealAssignment>{let(a) = x + y}),
          grid(2) {
    }

    Void test_augmented_paving_interface() {
        Mapping mapping = system.auxiliary_mapping();
        AugmentedPaving storage(grid, mapping);
        ARIADNE_TESTING_PROVE(storage.is_empty());
        ARIADNE_TESTING_EXPECT_EQUAL(storage.size(), 0u);
        ARIADNE_TESTING_EXPECT_EQUAL(storage.dimension(), 2u);
        ARIADNE_TESTING_PROVE(storage.grid() == grid);
        ARIADNE_TESTING_PROVE(storage.begin() == storage.end());

        ExactBoxType box({
            {-1, 1},
            {-2, 2}
        });
        RealExpressionBoundedConstraintSet symbolic_box({-1_dec <= x <= 1_dec, -2_dec <= y <= 2_dec});
        BoundedConstraintSet compact_set = symbolic_box.euclidean_set(system.state_space());
        ValidatedConstrainedImageSet validated_set(box);

        storage.adjoin_outer_approximation(box, 2u);
        ARIADNE_TESTING_PROVE(storage.size() > 0u);
        storage.adjoin_outer_approximation(compact_set, 2u);
        storage.adjoin_outer_approximation(validated_set, 2u);
        storage.adjoin_inner_approximation(compact_set, 2u);
        storage.adjoin_lower_approximation(compact_set, 4u, 3u);
        ARIADNE_TESTING_PROVE(storage.size() > 0u);

        AugmentedPaving other(grid, mapping);
        other.adjoin_outer_approximation(ExactBoxType({
                                             {0, 1},
                                             {0, 1}
        }),
                                         2u);
        ARIADNE_TESTING_PROVE(intersect(storage, other));
        ARIADNE_TESTING_PROVE(subset(other, storage));
        storage.adjoin(other);
        storage.restrict(other);
        storage.remove(other);
        ARIADNE_TESTING_PROVE(storage.is_empty());

        storage.adjoin_outer_approximation(box, 2u);
        storage.restrict_to_extent(4u);
        storage.mince(3u);
        storage.recombine();
        ARIADNE_TESTING_PROVE(storage.size() > 0u);

        Paving& paving = storage.state_paving();
        const AugmentedPaving& const_storage = storage;
        const Paving& const_paving = const_storage.state_paving();
        ARIADNE_TESTING_PROVE(&storage.state_set() == &storage.state_paving());
        ARIADNE_TESTING_PROVE(&const_storage.state_set() == &const_storage.state_paving());
        ARIADNE_TESTING_PROVE(paving.size() == const_paving.size());
        ARIADNE_TESTING_PROVE(const_storage.begin() != const_storage.end());

        BinaryTreePaving auxiliary = storage.state_auxiliary_paving();
        BinaryTreePaving legacy_auxiliary = storage.state_auxiliary_set();
        ARIADNE_TESTING_PROVE(subset(auxiliary, legacy_auxiliary));
        ARIADNE_TESTING_PROVE(subset(legacy_auxiliary, auxiliary));
        ARIADNE_TESTING_EXPECT_EQUAL(auxiliary.dimension(), 3u);
        ARIADNE_TESTING_EXPECT_EQUAL(storage.auxiliary_mapping().result_size(), mapping.result_size());
        ARIADNE_TESTING_EXPECT_EQUAL(storage.auxiliary_function().result_size(), mapping.result_size());
        ARIADNE_TESTING_EXPECT_EQUAL(storage.auxiliary_data().result_size(), mapping.result_size());

        std::unique_ptr<AugmentedPaving> copy(storage.clone());
        ARIADNE_TESTING_PROVE(copy->size() == storage.size());

        NullCanvas canvas;
        PavingDrawing::draw(canvas, Projection2d(3, 0, 1), storage);
        PavingDrawing::AugmentedPavingDrawable drawable(storage);
        const Drawable2dInterface& drawable_interface = drawable;
        drawable_interface.draw(canvas, Projection2d(3, 0, 1));
        std::unique_ptr<Drawable2dInterface> drawable_copy(drawable.clone());
        ARIADNE_TESTING_EXPECT_EQUAL(drawable_copy->dimension(), storage.state_auxiliary_paving().dimension());

        ARIADNE_TESTING_NOTE(storage);

        storage.clear();
        ARIADNE_TESTING_PROVE(storage.is_empty());
    }

    Void test_labelled_augmented_paving_value() {
        RealSpace state_space = system.state_space();
        RealSpace auxiliary_space = system.auxiliary_space();
        Mapping mapping = system.auxiliary_mapping();

        const auto lgrid = make_labelled_grid(grid, state_space);
        ARIADNE_TESTING_PROVE(lgrid.value() == grid);
        ARIADNE_TESTING_PROVE(lgrid.space() == state_space);

        auto ls1 = make_labelled_augmented_paving(
            AugmentedPaving(Paving(new BinaryTreePaving(grid)), mapping), state_space, auxiliary_space);
        const auto ls2 = make_labelled_augmented_paving(lgrid, system);
        const auto ls3 = make_labelled_augmented_paving(Paving(new BinaryTreePaving(grid)), system);

        ARIADNE_TESTING_PROVE(Ariadne::state_space(ls1) == state_space);
        ARIADNE_TESTING_PROVE(Ariadne::auxiliary_space(ls1) == auxiliary_space);
        ARIADNE_TESTING_PROVE(state_auxiliary_space(ls1).size() == 3u);
        ARIADNE_TESTING_PROVE(Ariadne::state_space(ls2) == state_space);
        ARIADNE_TESTING_PROVE(Ariadne::state_space(ls3) == state_space);
        ARIADNE_TESTING_PROVE(ls1.value().grid() == grid);
        ARIADNE_TESTING_EXPECT_EQUAL(ls1.value().auxiliary_mapping().result_size(), mapping.result_size());

        ExactBoxType box({
            {-1, 1},
            {-2, 2}
        });
        ls1.value().adjoin_outer_approximation(box, 2u);
        auto ls_other = make_labelled_augmented_paving(
            AugmentedPaving(Paving(new BinaryTreePaving(grid)), mapping), state_space, auxiliary_space);
        ls_other.value().adjoin_outer_approximation(ExactBoxType({
                                                        {0, 1},
                                                        {0, 1}
        }),
                                                    2u);
        ARIADNE_TESTING_PROVE(ls1.value().size() > 0u);
        ARIADNE_TESTING_PROVE(subset(ls_other, ls1));
        ARIADNE_TESTING_PROVE(intersect(ls1, ls_other));
        adjoin(ls1, ls_other);
        restrict(ls1, ls_other);
        remove(ls1, ls_other);
        ARIADNE_TESTING_PROVE(ls1.value().is_empty());

        ls1.value().adjoin_outer_approximation(box, 2u);
        const LabelledAugmentedPaving copy(ls1);
        ARIADNE_TESTING_PROVE(copy.value().size() == ls1.value().size());
        PavingDrawing::LabelledAugmentedPavingValueDrawable drawable(ls1);
        std::unique_ptr<LabelledDrawable2dInterface> lcopy(drawable.labelled_clone());
        ARIADNE_TESTING_PROVE(lcopy != nullptr);

        NullCanvas canvas;
        PavingDrawing::draw(canvas, Variables2d(x, y), ls1);
        lcopy->draw(canvas, Variables2d(x, y));

        // Exercise the real labelled-figure consumer with the generic value.
        LabelledFigure figure(Axes2d(-2.0, x, 2.0, -2.0, y, 2.0));
        PavingDrawing::draw(figure, ls1);
        ARIADNE_TESTING_EXPECT_NO_THROW(figure._paint_all(canvas));

        RealExpressionBoundedConstraintSet symbolic_set({-1_dec <= x <= 1_dec, -2_dec <= y <= 2_dec});
        BoundedConstraintSet set = symbolic_set.euclidean_set(state_space);
        const Paving inner = inner_approximation(set, lgrid, 2u);
        ARIADNE_TESTING_PROVE(inner.size() > 0u);
    }

    Void test_labelled_augmented_paving_value_preconditions() {
        RealVariable u("u"), v("v");
        RealSpace other_state_space({u, v});
        RealSpace state_space = system.state_space();
        RealSpace auxiliary_space = system.auxiliary_space();
        Mapping mapping = system.auxiliary_mapping();

        // Grid/mapping dimension mismatch.
        ARIADNE_TESTING_EXPECT_EXCEPTION(make_labelled_augmented_paving(
            AugmentedPaving(Paving(new BinaryTreePaving(Grid(3))), mapping), state_space, auxiliary_space));
        // Labelled grid whose space differs from the system state space.
        const auto other_lgrid = make_labelled_grid(grid, other_state_space);
        ARIADNE_TESTING_EXPECT_EXCEPTION(make_labelled_augmented_paving(other_lgrid, system));
        VectorField other_system(List<DottedRealAssignment>{dot(u) = -u, dot(v) = -v});
        const auto lgrid = make_labelled_grid(grid, state_space);
        ARIADNE_TESTING_EXPECT_EXCEPTION(make_labelled_augmented_paving(lgrid, other_system));

        // Operations between values with different state variables.
        auto ls = make_labelled_augmented_paving(
            AugmentedPaving(Paving(new BinaryTreePaving(grid)), mapping), state_space, auxiliary_space);
        const auto other_ls = make_labelled_augmented_paving(
            AugmentedPaving(Paving(new BinaryTreePaving(grid)), mapping), other_state_space, auxiliary_space);
        ARIADNE_TESTING_EXPECT_EXCEPTION(adjoin(ls, other_ls));
        ARIADNE_TESTING_EXPECT_EXCEPTION(restrict(ls, other_ls));
        ARIADNE_TESTING_EXPECT_EXCEPTION(remove(ls, other_ls));
        ARIADNE_TESTING_EXPECT_EXCEPTION(subset(ls, other_ls));
        ARIADNE_TESTING_EXPECT_EXCEPTION(intersect(ls, other_ls));
    }

    Void test_augmented_paving_orbit() {
        Mapping mapping = system.auxiliary_mapping();
        AugmentedPaving initial(grid, mapping);
        initial.adjoin_outer_approximation(ExactBoxType({
                                               {0, 1},
                                               {0, 1}
        }),
                                           2u);

        Orbit<AugmentedPaving> orbit(initial);
        ARIADNE_TESTING_PROVE(orbit.initial().size() == initial.size());
        ARIADNE_TESTING_PROVE(orbit.reach().is_empty());
        ARIADNE_TESTING_PROVE(orbit.intermediate().is_empty());
        ARIADNE_TESTING_PROVE(orbit.final().is_empty());

        AugmentedPaving reach(grid, mapping);
        reach.adjoin_outer_approximation(ExactBoxType({
                                             {0, 2},
                                             {0, 2}
        }),
                                         2u);
        AugmentedPaving intermediate = reach;
        AugmentedPaving final_set = reach;
        Orbit<AugmentedPaving> full_orbit(initial, reach, intermediate, final_set);
        ARIADNE_TESTING_PROVE(full_orbit.initial().size() == initial.size());
        ARIADNE_TESTING_PROVE(full_orbit.reach().size() == reach.size());
        ARIADNE_TESTING_PROVE(full_orbit.intermediate().size() == intermediate.size());
        ARIADNE_TESTING_PROVE(full_orbit.final().size() == final_set.size());
    }

    Void test_labelled_value_drawing() {
        RealVariable state_x("x"), state_y("y"), aux_z("z");
        RealSpace state({state_x, state_y}), auxiliary({aux_z});
        EffectiveScalarMultivariateFunction coordinate =
            EffectiveScalarMultivariateFunction::coordinate(2u, 0u);
        EffectiveVectorMultivariateFunction mapping({coordinate});
        LabelledMapping labelled_mapping(state, mapping, auxiliary);
        const auto labelled_grid = make_labelled_grid(Grid(2u), state);
        auto paving = make_labelled_augmented_paving(
            AugmentedPaving(Paving(new BinaryTreePaving(labelled_grid.value())), labelled_mapping.mapping()),
            state, auxiliary);
        ARIADNE_TESTING_PROVE(paving.value().is_empty());
        adjoin_outer_approximation(paving, ExactBoxType({
                                               {-1, 1},
                                               {-1, 1}
        }),
                                   2u);
        ARIADNE_TESTING_PROVE(paving.value().size() > 0u);
        ARIADNE_TESTING_EXPECT_EQUAL(state_space(paving), state);
        ARIADNE_TESTING_EXPECT_EQUAL(auxiliary_space(paving), auxiliary);
        ARIADNE_TESTING_EXPECT_EQUAL(paving.value().state_auxiliary_paving().dimension(), 3u);

        NullCanvas canvas;
        PavingDrawing::draw(canvas, Variables2d(state_x, state_y), paving);
        PavingDrawing::LabelledAugmentedPavingValueDrawable drawable(paving);
        drawable.draw(canvas, Variables2d(state_x, state_y));
        std::unique_ptr<LabelledDrawable2dInterface> clone(drawable.labelled_clone());
        clone->draw(canvas, Variables2d(state_x, state_y));

        Figure figure(ExactBoxType({
                          {-2, 2},
                          {-2, 2},
                          {-2, 2}
        }),
                      Projection2d(3u, 0u, 1u));
        PavingDrawing::draw(figure, paving);
        LabelledFigure labelled_figure(
            Axes2d(-2.0, state_x, 2.0, -2.0, state_y, 2.0));
        PavingDrawing::draw(labelled_figure, paving);
        ARIADNE_TESTING_EXPECT_NO_THROW(labelled_figure._paint_all(canvas));
    }

    Void test_enclosure_augmented_paving() {
        GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-2_pr));
        VectorFieldEvolver evolver(system, integrator);

        ExactBoxType box({
            {0, 2},
            {0, 2}
        });
        Enclosure enclosure = evolver.enclosure(box);
        // Attach the same auxiliary mapping as the system, so that adjoining
        // the enclosure to storage built from the system triggers the
        // consistency warning path.
        enclosure.set_auxiliary_mapping(system.auxiliary_mapping());

        AugmentedPaving storage(grid, system.auxiliary_mapping());
        adjoin_outer_approximation_to(enclosure, storage, 2u, Paver(new AffinePaver()));
        ARIADNE_TESTING_PROVE(storage.size() > 0u);

        // Same-sized but distinct auxiliary mapping: the consistency check
        // evaluates the pointer comparison as false.
        VectorField twin_system(List<DottedRealAssignment>{dot(x) = -x, dot(y) = -y},
                                List<RealAssignment>{let(a) = x + y});
        AugmentedPaving twin_storage(grid, twin_system.auxiliary_mapping());
        adjoin_outer_approximation_to(enclosure, twin_storage, 2u, Paver(new AffinePaver()));
        ARIADNE_TESTING_PROVE(twin_storage.size() > 0u);

        AugmentedPaving approximation = outer_approximation(enclosure, grid, 2u, Paver(new AffinePaver()));
        ARIADNE_TESTING_PROVE(approximation.size() > 0u);
        ARIADNE_TESTING_PROVE(subset(storage, approximation) || intersect(storage, approximation));
    }
};

int main(int argc, char** argv) {
    DrawerManager::instance().set_drawer(BoxDrawer());

    ARIADNE_TESTING_SUITE("test_augmented_paving");
    ARIADNE_TESTING_ADD_TEST(TestAugmentedPaving, test_augmented_paving_interface);
    ARIADNE_TESTING_ADD_TEST(TestAugmentedPaving, test_labelled_augmented_paving_value);
    ARIADNE_TESTING_ADD_TEST(TestAugmentedPaving, test_labelled_augmented_paving_value_preconditions);
    ARIADNE_TESTING_ADD_TEST(TestAugmentedPaving, test_augmented_paving_orbit);
    ARIADNE_TESTING_ADD_TEST(TestAugmentedPaving, test_labelled_value_drawing);
    ARIADNE_TESTING_ADD_TEST(TestAugmentedPaving, test_enclosure_augmented_paving);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

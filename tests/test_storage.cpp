/***************************************************************************
 *            test_storage.cpp
 *
 *  Copyright  2006-20  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne.
 *
 *  Ariadne is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <memory>

#include "utility/tuple.hpp"
#include "numeric/numeric.hpp"
#include "algebra/vector.hpp"
#include "function/function.hpp"
#include "geometry/box.hpp"
#include "set/list_set.hpp"
#include "set/function_set.hpp"
#include "paving/grid_paving.hpp"
#include "expression-set/expression_set.hpp"
#include "evolution/enclosure.hpp"
#include "evolution/orbit.hpp"
#include "integration/integrator.hpp"
#include "system/vector_field.hpp"
#include "evolution/vector_field_evolver.hpp"
#include "rendering/null_rendering.hpp"
#include "labelled/figure.hpp"
#include "drawing/drawer_manager.hpp"
#include "drawing/drawer.hpp"

#include "reasoning/storage.hpp"
#include "reasoning/storage_orbit.hpp"
#include "reasoning/enclosure_storage.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;
using namespace std;


class TestStorage : public Ariadne::Testing::ContextFixture
{
    RealVariable x;
    RealVariable y;
    RealVariable a;
    VectorField system;
    Grid grid;
  public:
    using ContextFixture::ContextFixture;

    TestStorage(Ariadne::Testing::Context& context)
        : ContextFixture(context),
          x("x"), y("y"), a("a"),
          system(List<DottedRealAssignment>{dot(x)=-x, dot(y)=-y},
                 List<RealAssignment>{let(a)=x+y}),
          grid(2)
    {
    }

    Void test_storage_interface() {
        Mapping mapping = system.auxiliary_mapping();
        Storage storage(grid, mapping);
        ARIADNE_TESTING_PROVE(storage.is_empty());
        ARIADNE_TESTING_EXPECT_EQUAL(storage.size(), 0u);
        ARIADNE_TESTING_EXPECT_EQUAL(storage.dimension(), 2u);
        ARIADNE_TESTING_PROVE(storage.grid() == grid);
        ARIADNE_TESTING_PROVE(storage.begin() == storage.end());

        ExactBoxType box({{-1,1},{-2,2}});
        RealExpressionBoundedConstraintSet symbolic_box({-1_dec<=x<=1_dec, -2_dec<=y<=2_dec});
        BoundedConstraintSet compact_set = symbolic_box.euclidean_set(system.state_space());
        ValidatedConstrainedImageSet validated_set(box);

        storage.adjoin_outer_approximation(box, 2u);
        ARIADNE_TESTING_PROVE(storage.size() > 0u);
        storage.adjoin_outer_approximation(compact_set, 2u);
        storage.adjoin_outer_approximation(validated_set, 2u);
        storage.adjoin_inner_approximation(compact_set, 2u);
        storage.adjoin_lower_approximation(compact_set, 4u, 3u);
        ARIADNE_TESTING_PROVE(storage.size() > 0u);

        Storage other(grid, mapping);
        other.adjoin_outer_approximation(ExactBoxType({{0,1},{0,1}}), 2u);
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

        GridTreePaving& paving = storage.state_set();
        const Storage& const_storage = storage;
        const GridTreePaving& const_paving = const_storage.state_set();
        ARIADNE_TESTING_PROVE(paving.size() == const_paving.size());
        ARIADNE_TESTING_PROVE(const_storage.begin() != const_storage.end());

        GridTreePaving auxiliary = storage.state_auxiliary_set();
        ARIADNE_TESTING_EXPECT_EQUAL(auxiliary.dimension(), 3u);
        ARIADNE_TESTING_EXPECT_EQUAL(storage.auxiliary_mapping().result_size(), mapping.result_size());
        ARIADNE_TESTING_EXPECT_EQUAL(storage.auxiliary_function().result_size(), mapping.result_size());
        ARIADNE_TESTING_EXPECT_EQUAL(storage.auxiliary_data().result_size(), mapping.result_size());

        std::unique_ptr<Storage> copy(storage.clone());
        ARIADNE_TESTING_PROVE(copy->size() == storage.size());

        NullCanvas canvas;
        storage.draw(canvas, Projection2d(2, 0, 1));

        ARIADNE_TESTING_NOTE(storage);

        storage.clear();
        ARIADNE_TESTING_PROVE(storage.is_empty());
    }

    Void test_labelled_storage() {
        RealSpace state_space = system.state_space();
        RealSpace auxiliary_space = system.auxiliary_space();
        Mapping mapping = system.auxiliary_mapping();

        LabelledGrid lgrid(grid, state_space);
        ARIADNE_TESTING_PROVE(lgrid.euclidean_grid() == grid);
        ARIADNE_TESTING_PROVE(lgrid.space() == state_space);
        ARIADNE_TESTING_NOTE(lgrid);

        LabelledGridTreePaving lgtp(grid, state_space);
        LabelledGridTreePaving lgtp_from_paving(GridTreePaving(grid), state_space);
        LabelledGridTreePaving lgtp_from_lgrid(lgrid);
        ARIADNE_TESTING_PROVE(lgtp.euclidean_set().grid() == grid);
        ARIADNE_TESTING_PROVE(lgtp.space() == state_space);
        const LabelledGridTreePaving& const_lgtp = lgtp;
        ARIADNE_TESTING_PROVE(const_lgtp.euclidean_set().grid() == grid);

        LabelledStateAuxiliaryGrid lsag(grid, state_space, mapping, auxiliary_space);
        LabelledStateAuxiliaryGrid lsag_from_system(system);
        ARIADNE_TESTING_PROVE(lsag.euclidean_grid() == grid);
        ARIADNE_TESTING_EXPECT_EQUAL(lsag.auxiliary_mapping().result_size(), mapping.result_size());
        ARIADNE_TESTING_PROVE(lsag.state_space() == state_space);
        ARIADNE_TESTING_PROVE(lsag.auxiliary_space() == auxiliary_space);
        ARIADNE_TESTING_PROVE(lsag_from_system.state_space() == state_space);
        ARIADNE_TESTING_NOTE(lsag);

        LabelledMapping lmapping(state_space, mapping, auxiliary_space);
        ARIADNE_TESTING_PROVE(lmapping.argument_space() == state_space);
        ARIADNE_TESTING_EXPECT_EQUAL(lmapping.mapping().result_size(), mapping.result_size());
        ARIADNE_TESTING_EXPECT_EQUAL(lmapping.function().result_size(), mapping.result_size());
        ARIADNE_TESTING_PROVE(lmapping.result_space() == auxiliary_space);

        LabelledStorage ls1(grid, state_space, mapping, auxiliary_space);
        LabelledStorage ls2(GridTreePaving(grid), state_space, mapping, auxiliary_space);
        LabelledStorage ls3(grid, lmapping);
        LabelledStorage ls4(GridTreePaving(grid), lmapping);
        LabelledStorage ls5(lgrid, lmapping);
        LabelledStorage ls6(lgtp_from_lgrid, lmapping);
        LabelledStorage ls7(grid, system);
        LabelledStorage ls8(GridTreePaving(grid), system);
        LabelledStorage ls9(lgrid, system);
        LabelledStorage ls10(lgtp_from_lgrid, system);

        ARIADNE_TESTING_PROVE(ls1.state_space() == state_space);
        ARIADNE_TESTING_PROVE(ls1.auxiliary_space() == auxiliary_space);
        ARIADNE_TESTING_PROVE(ls1.state_auxiliary_space().size() == 3u);
        ARIADNE_TESTING_PROVE(ls1.grid().space() == state_space);
        ARIADNE_TESTING_EXPECT_EQUAL(ls1.auxiliary_mapping().mapping().result_size(), mapping.result_size());
        ARIADNE_TESTING_EXPECT_EQUAL(ls1.auxiliary_data().mapping().result_size(), mapping.result_size());
        ARIADNE_TESTING_EXPECT_EQUAL(ls1.auxiliary_function().result_size(), mapping.result_size());
        Storage& euclidean = ls1.euclidean_set();
        const LabelledStorage& const_ls = ls1;
        const Storage& const_euclidean = const_ls.euclidean_set();
        ARIADNE_TESTING_PROVE(euclidean.grid() == const_euclidean.grid());

        ExactBoxType box({{-1,1},{-2,2}});
        ls1.adjoin_outer_approximation(box, 2u);
        ls2.adjoin_outer_approximation(box, 2u);
        ls3.adjoin_outer_approximation(ExactBoxType({{0,1},{0,1}}), 2u);
        ARIADNE_TESTING_PROVE(ls1.size() > 0u);
        ARIADNE_TESTING_PROVE(subset(ls3, ls1));
        ARIADNE_TESTING_PROVE(intersect(ls1, ls2));
        ls1.adjoin(ls2);
        ls1.restrict(ls2);
        ls1.remove(ls2);
        ARIADNE_TESTING_PROVE(ls1.is_empty());

        ls1.adjoin_outer_approximation(box, 2u);
        std::unique_ptr<LabelledStorage> copy(ls1.clone());
        std::unique_ptr<LabelledStorage> lcopy(ls1.labelled_clone());
        ARIADNE_TESTING_PROVE(copy->size() == ls1.size());
        ARIADNE_TESTING_PROVE(lcopy->size() == ls1.size());

        NullCanvas canvas;
        ls1.draw(canvas, Variables2d(x, y));

        ARIADNE_TESTING_NOTE(ls1);

        RealExpressionBoundedConstraintSet symbolic_set({-1_dec<=x<=1_dec, -2_dec<=y<=2_dec});
        BoundedConstraintSet set = symbolic_set.euclidean_set(state_space);
        LabelledGridTreePaving inner = inner_approximation(set, lgrid, 2u);
        ARIADNE_TESTING_PROVE(inner.euclidean_set().size() > 0u);
    }

    Void test_labelled_storage_preconditions() {
        RealVariable u("u"), v("v");
        RealSpace other_state_space({u, v});
        RealSpace state_space = system.state_space();
        RealSpace auxiliary_space = system.auxiliary_space();
        Mapping mapping = system.auxiliary_mapping();
        LabelledMapping lmapping(state_space, mapping, auxiliary_space);

        // Grid/mapping dimension mismatch.
        ARIADNE_TESTING_EXPECT_EXCEPTION(LabelledStorage(Grid(3), lmapping));
        ARIADNE_TESTING_EXPECT_EXCEPTION(LabelledStorage(GridTreePaving(Grid(3)), lmapping));
        // Labelled grid/paving whose space differs from the mapping argument space.
        LabelledGrid other_lgrid(grid, other_state_space);
        LabelledGridTreePaving other_lgtp(other_lgrid);
        ARIADNE_TESTING_EXPECT_EXCEPTION(LabelledStorage(other_lgrid, lmapping));
        ARIADNE_TESTING_EXPECT_EXCEPTION(LabelledStorage(other_lgtp, lmapping));
        // System whose state space differs from the labelled grid/paving.
        VectorField other_system(List<DottedRealAssignment>{dot(u)=-u, dot(v)=-v});
        LabelledGrid lgrid(grid, state_space);
        LabelledGridTreePaving lgtp(lgrid);
        ARIADNE_TESTING_EXPECT_EXCEPTION(LabelledStorage(lgrid, other_system));
        ARIADNE_TESTING_EXPECT_EXCEPTION(LabelledStorage(lgtp, other_system));

        // Operations between storages with different state variables.
        LabelledStorage ls(grid, state_space, mapping, auxiliary_space);
        LabelledStorage other_ls(grid, other_state_space, mapping, auxiliary_space);
        ARIADNE_TESTING_EXPECT_EXCEPTION(ls.adjoin(other_ls));
        ARIADNE_TESTING_EXPECT_EXCEPTION(ls.restrict(other_ls));
        ARIADNE_TESTING_EXPECT_EXCEPTION(ls.remove(other_ls));
        ARIADNE_TESTING_EXPECT_EXCEPTION(subset(ls, other_ls));
        ARIADNE_TESTING_EXPECT_EXCEPTION(intersect(ls, other_ls));
    }

    Void test_storage_orbit() {
        Mapping mapping = system.auxiliary_mapping();
        Storage initial(grid, mapping);
        initial.adjoin_outer_approximation(ExactBoxType({{0,1},{0,1}}), 2u);

        Orbit<Storage> orbit(initial);
        ARIADNE_TESTING_PROVE(orbit.initial().size() == initial.size());
        ARIADNE_TESTING_PROVE(orbit.reach().is_empty());
        ARIADNE_TESTING_PROVE(orbit.intermediate().is_empty());
        ARIADNE_TESTING_PROVE(orbit.final().is_empty());

        Storage reach(grid, mapping);
        reach.adjoin_outer_approximation(ExactBoxType({{0,2},{0,2}}), 2u);
        Storage intermediate = reach;
        Storage final_set = reach;
        Orbit<Storage> full_orbit(initial, reach, intermediate, final_set);
        ARIADNE_TESTING_PROVE(full_orbit.initial().size() == initial.size());
        ARIADNE_TESTING_PROVE(full_orbit.reach().size() == reach.size());
        ARIADNE_TESTING_PROVE(full_orbit.intermediate().size() == intermediate.size());
        ARIADNE_TESTING_PROVE(full_orbit.final().size() == final_set.size());
    }

    Void test_enclosure_storage() {
        GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-2_pr));
        VectorFieldEvolver evolver(system, integrator);

        ExactBoxType box({{0,2},{0,2}});
        Enclosure enclosure = evolver.enclosure(box);
        // Attach the same auxiliary mapping as the system, so that adjoining
        // the enclosure to storage built from the system triggers the
        // consistency warning path.
        enclosure.set_auxiliary_mapping(system.auxiliary_mapping());

        Storage storage(grid, system.auxiliary_mapping());
        adjoin_outer_approximation_to(enclosure, storage, 2u);
        ARIADNE_TESTING_PROVE(storage.size() > 0u);

        // Same-sized but distinct auxiliary mapping: the consistency check
        // evaluates the pointer comparison as false.
        VectorField twin_system(List<DottedRealAssignment>{dot(x)=-x, dot(y)=-y},
                                List<RealAssignment>{let(a)=x+y});
        Storage twin_storage(grid, twin_system.auxiliary_mapping());
        adjoin_outer_approximation_to(enclosure, twin_storage, 2u);
        ARIADNE_TESTING_PROVE(twin_storage.size() > 0u);

        Storage approximation = outer_approximation(enclosure, grid, 2u);
        ARIADNE_TESTING_PROVE(approximation.size() > 0u);
        ARIADNE_TESTING_PROVE(subset(storage, approximation) || intersect(storage, approximation));
    }
};


int main(int argc, char** argv)
{
    DrawerManager::instance().set_drawer(GridDrawer(2));

    ARIADNE_TESTING_SUITE("test_storage");
    ARIADNE_TESTING_ADD_TEST(TestStorage, test_storage_interface);
    ARIADNE_TESTING_ADD_TEST(TestStorage, test_labelled_storage);
    ARIADNE_TESTING_ADD_TEST(TestStorage, test_labelled_storage_preconditions);
    ARIADNE_TESTING_ADD_TEST(TestStorage, test_storage_orbit);
    ARIADNE_TESTING_ADD_TEST(TestStorage, test_enclosure_storage);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}

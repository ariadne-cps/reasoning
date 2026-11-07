/***************************************************************************
 *            test_hybrid_augmented_paving.cpp
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
#include "system/hybrid_automata.hpp"
#include "hybrid/hybrid_set.hpp"
#include "hybrid/hybrid_paving.hpp"
#include "rendering/null_rendering.hpp"
#include "labelled/figure.hpp"
#include "drawing/drawer_manager.hpp"
#include "drawing/drawer.hpp"

#include "reasoning/hybrid_auxiliary_mapping.hpp"
#include "reasoning/hybrid_augmented_paving.hpp"
#include "reasoning/hybrid_augmented_paving_draw.hpp"
#include "reasoning/analysis_paving_factory.hpp"
#include "reasoning/hybrid_augmented_paving_orbit.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;
using namespace std;


class TestHybridAugmentedPaving : public Ariadne::Testing::ContextFixture
{
    RealVariable x;
    RealVariable y;
    DiscreteLocation q;
    HybridAutomaton system;
  public:
    using ContextFixture::ContextFixture;

    TestHybridAugmentedPaving(Ariadne::Testing::Context& context)
        : ContextFixture(context),
          x("x"), y("y"), q(),
          system()
    {
        system.new_mode(q, {dot(x)=-x, dot(y)=-y});
    }

    Void test_hybrid_augmented_paving_interface() {
        HybridGrid grid(system.state_space());
        HybridGridTreePaving hgtp(grid);
        HybridAugmentedPaving from_grid = make_analysis_paving<HybridAugmentedPaving>(grid,system);
        // Construct independently from the originating automaton.
        LabelledMapping explicit_mapping(system.continuous_state_space(q),
                                         system.auxiliary_function(q),
                                         system.continuous_auxiliary_space(q));
        HybridAuxiliaryMapping auxiliary(system.state_auxiliary_space(),
            [explicit_mapping](DiscreteLocation const&) { return explicit_mapping; });
        HybridAugmentedPaving independent(grid,auxiliary);
        ARIADNE_TESTING_PROVE(independent.auxiliary_mapping(q).argument_space() ==
                              system.continuous_state_space(q));
        HybridAugmentedPaving from_paving = make_analysis_paving<HybridAugmentedPaving>(hgtp,system);

        ARIADNE_TESTING_PROVE(from_grid.is_empty());
        ARIADNE_TESTING_EXPECT_EQUAL(from_grid.size(), 0u);
        ARIADNE_TESTING_PROVE(from_grid.begin() == from_grid.end());
        ARIADNE_TESTING_PROVE(from_grid.auxiliary_data().state_auxiliary_space()[q] == system.state_auxiliary_space()[q]);
        ARIADNE_TESTING_PROVE(from_grid.grid().space() == grid.space());
        ARIADNE_TESTING_PROVE(from_grid.auxiliary_mapping(q).argument_space() == system.continuous_state_space(q));

        HybridBoundedConstraintSet initial(q, {0.9_dec<=x<=1.1_dec, 0.9_dec<=y<=1.1_dec});
        HybridExactBoxes boxes;
        boxes.insert(q, system.continuous_state_space(q), ExactBoxType({{-2,2},{-2,2}}));

        from_grid.adjoin_outer_approximation(initial, 2u);
        ARIADNE_TESTING_PROVE(from_grid.size() > 0u);
        from_grid.adjoin_outer_approximation(boxes, 2u);
        from_grid.adjoin_lower_approximation(initial, 4u, 3u);
        ARIADNE_TESTING_PROVE(from_grid.size() > 0u);

        HybridGridTreePaving& state_set = from_grid.state_set();
        const HybridAugmentedPaving& const_storage = from_grid;
        const HybridGridTreePaving& const_state_set = const_storage.state_set();
        ARIADNE_TESTING_PROVE(state_set.size() == const_state_set.size());
        ARIADNE_TESTING_PROVE(const_storage.begin() != const_storage.end());

        HybridGridTreePaving auxiliary_set = from_grid.state_auxiliary_paving();
        ARIADNE_TESTING_PROVE(auxiliary_set.size() > 0u);
        independent.adjoin_outer_approximation(initial,2u);
        ARIADNE_TESTING_PROVE(independent.state_auxiliary_paving().size() > 0u);
        ARIADNE_TESTING_PROVE(independent.state_auxiliary_paving().space(q) ==
                              system.state_auxiliary_space()[q]);
        HybridAugmentedPaving const& const_independent=independent;
        LabelledAugmentedPaving labelled_view=const_independent[q];
        ARIADNE_TESTING_PROVE(labelled_view.size()==independent.size());

        ARIADNE_TESTING_PROVE(subset(from_paving, from_grid));
        from_paving.adjoin(from_grid);
        ARIADNE_TESTING_PROVE(from_paving.size() == from_grid.size());
        ARIADNE_TESTING_PROVE(intersect(from_paving, from_grid));
        from_paving.restrict(from_grid);
        from_paving.remove(from_grid);
        ARIADNE_TESTING_PROVE(from_paving.is_empty());

        from_grid.restrict_to_extent(4u);
        from_grid.mince(3u);
        from_grid.recombine();
        ARIADNE_TESTING_PROVE(from_grid.size() > 0u);

        auto element = from_grid[q];
        LabelledAugmentedPaving labelled = element;
        ARIADNE_TESTING_PROVE(labelled.size() == from_grid.size());
        element.clear();
        ARIADNE_TESTING_PROVE(from_grid.is_empty());

        from_grid.adjoin_outer_approximation(initial, 2u);
        NullCanvas canvas;
        Set<DiscreteLocation> locations{q};
        draw(canvas, locations, Variables2d(x, y), from_grid);
        HybridAugmentedPavingDrawable drawable(from_grid);
        HybridDrawableInterface const& drawable_interface=drawable;
        drawable_interface.draw(canvas,locations,Variables2d(x,y));

        ARIADNE_TESTING_NOTE(from_grid);

        from_grid.clear();
        ARIADNE_TESTING_PROVE(from_grid.is_empty());
    }

    Void test_hybrid_augmented_paving_orbit() {
        HybridGrid grid(system.state_space());
        HybridAugmentedPaving initial = make_analysis_paving<HybridAugmentedPaving>(grid,system);
        HybridBoundedConstraintSet initial_set(q, {0.9_dec<=x<=1.1_dec, 0.9_dec<=y<=1.1_dec});
        initial.adjoin_outer_approximation(initial_set, 2u);

        Orbit<HybridAugmentedPaving> orbit(initial);
        ARIADNE_TESTING_PROVE(orbit.initial().size() == initial.size());
        ARIADNE_TESTING_PROVE(orbit.reach().is_empty());
        ARIADNE_TESTING_PROVE(orbit.intermediate().is_empty());
        ARIADNE_TESTING_PROVE(orbit.final().is_empty());

        HybridAugmentedPaving reach = make_analysis_paving<HybridAugmentedPaving>(grid,system);
        reach.adjoin_outer_approximation(initial_set, 2u);
        HybridAugmentedPaving intermediate = reach;
        HybridAugmentedPaving final_set = reach;

        Orbit<HybridAugmentedPaving> full_orbit(initial, reach, intermediate, final_set);
        ARIADNE_TESTING_PROVE(full_orbit.initial().size() == initial.size());
        ARIADNE_TESTING_PROVE(full_orbit.reach().size() == reach.size());
        ARIADNE_TESTING_PROVE(full_orbit.intermediate().size() == intermediate.size());
        ARIADNE_TESTING_PROVE(full_orbit.final().size() == final_set.size());

        NullCanvas canvas;
        draw(canvas, Set<DiscreteLocation>{q}, Variables2d(x, y), full_orbit);
    }
};


int main(int argc, char** argv)
{
    DrawerManager::instance().set_drawer(GridDrawer(2));

    ARIADNE_TESTING_SUITE("test_hybrid_augmented_paving");
    ARIADNE_TESTING_ADD_TEST(TestHybridAugmentedPaving, test_hybrid_augmented_paving_interface);
    ARIADNE_TESTING_ADD_TEST(TestHybridAugmentedPaving, test_hybrid_augmented_paving_orbit);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}

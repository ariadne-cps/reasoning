/***************************************************************************
 *            tests/test_hybrid_augmented_paving.cpp
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

/** @file tests/test_hybrid_augmented_paving.cpp
 *  @brief Generic HybridAugmentedPavingValue snapshots, orbits and flattened drawing.
 */

#include <iostream>
#include <memory>
#include <sstream>

#include "drawing/drawer.hpp"
#include "drawing/drawer_manager.hpp"
#include "function/function.hpp"
#include "geometry/box.hpp"
#include "hybrid/hybrid_auxiliary_mapping.hpp"
#include "hybrid/hybrid_bounded_constraint_set.hpp"
#include "hybrid/hybrid_set.hpp"
#include "labelled/figure.hpp"
#include "logging/test_runner.hpp"
#include "numeric/numeric.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/robdd_paving.hpp"
#include "reasoning/hybrid_augmented_paving_orbit.hpp"
#include "reasoning/hybrid_augmented_paving_value.hpp"
#include "reasoning/hybrid_augmented_paving_value_drawing.hpp"
#include "reasoning/hybrid_grid_value.hpp"
#include "reasoning/labelled_paving_value.hpp"
#include "reasoning/paving_drawing.hpp"
#include "rendering/null_rendering.hpp"
#include "set/list_set.hpp"
#include "system/hybrid_automata.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;
using namespace std;

// Count the Euclidean boundaries emitted by a location-filtered rendering.
class FlattenedPavingCanvas final : public NullCanvas {
  public:
    Nat boundaries = 0u;
    Void fill_boundary(const List<Point2d>&) override { ++boundaries; }
};

class TestHybridAugmentedPaving : public Ariadne::Testing::ContextFixture {
    RealVariable x;
    RealVariable y;
    DiscreteLocation q;
    HybridAutomaton system;

    static Hybrid<Scalings> unit_scalings(const HybridSpace& space) {
        return Hybrid<Scalings>(space, [](const DiscreteLocation&, const RealSpace&) {
            return Scalings(1.0_x, Map<RealVariable, ExactDouble>{});
        });
    }

    HybridAugmentedPavingValue make_value(const std::function<Paving(const Grid&)>& backend) const {
        Hybrid<Grid> grids = make_hybrid_grids(system.state_space(), unit_scalings(system.state_space()));
        return make_hybrid_augmented_paving_value(grids, system, backend);
    }

  public:
    using ContextFixture::ContextFixture;

    TestHybridAugmentedPaving(Ariadne::Testing::Context& context)
        : ContextFixture(context),
          x("x"), y("y"), q(),
          system() {
        system.new_mode(q, {dot(x) = -x, dot(y) = -y});
    }

    Void test_hybrid_augmented_paving_value_interface() {
        auto manager = std::make_shared<RobddManager>();
        std::function<Paving(const Grid&)> robdd_backend =
            [manager](const Grid& selected) { return Paving(new RobddPaving(manager, selected)); };
        std::function<Paving(const Grid&)> tree_backend =
            [](const Grid& selected) { return Paving(new BinaryTreePaving(selected)); };

        HybridAugmentedPavingValue storage = make_value(robdd_backend);
        ARIADNE_TESTING_PROVE(Ariadne::is_empty(storage));
        ARIADNE_TESTING_EXPECT_EQUAL(storage.size(), 0u);
        ARIADNE_TESTING_PROVE(storage.begin() == storage.end());
        ARIADNE_TESTING_PROVE(storage.space() == system.state_space());

        HybridBoundedConstraintSet initial(q, {0.9_dec <= x <= 1.1_dec, 0.9_dec <= y <= 1.1_dec});
        HybridExactBoxes boxes;
        boxes.insert(q, system.continuous_state_space(q), ExactBoxType({
                                                              {-2, 2},
                                                              {-2, 2}
        }));

        Ariadne::adjoin_outer_approximation(storage, initial, 2u);
        ARIADNE_TESTING_PROVE(storage.size() > 0u);
        Ariadne::adjoin_outer_approximation(storage, boxes, 2u);
        Ariadne::adjoin_lower_approximation(storage, initial, 4u, 3u);
        ARIADNE_TESTING_PROVE(storage.size() > 0u);
        ARIADNE_TESTING_PROVE(storage[q].size() > 0u);
        // Backend selection survives in the materialized slice.
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const RobddPaving*>(&static_cast<const PavingInterface&>(
                storage[q].state_paving())) != nullptr);

        // Read-only slice as an owning generic labelled value snapshot.
        const LabelledAugmentedPavingValue slice =
            make_labelled_augmented_paving(storage[q], system.continuous_state_space(q),
                                           system.continuous_auxiliary_space(q));
        const LabelledAugmentedPavingValue copied_slice(slice);
        ARIADNE_TESTING_EXPECT_EQUAL(state_space(slice), system.continuous_state_space(q));
        ARIADNE_TESTING_EXPECT_EQUAL(auxiliary_space(slice), system.continuous_auxiliary_space(q));
        ARIADNE_TESTING_PROVE(copied_slice.value().size() > 0u);
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const RobddPaving*>(
                &static_cast<const PavingInterface&>(
                    copied_slice.value().state_paving())) != nullptr);

        HybridAugmentedPavingValue other = make_value(robdd_backend);
        ARIADNE_TESTING_PROVE(Ariadne::subset(other, storage));
        Ariadne::adjoin(other, storage);
        ARIADNE_TESTING_PROVE(other.size() == storage.size());
        ARIADNE_TESTING_PROVE(Ariadne::intersect(other, storage));
        Ariadne::restrict(other, storage);
        Ariadne::remove(other, storage);
        ARIADNE_TESTING_PROVE(Ariadne::is_empty(other));

        Ariadne::restrict_to_extent(storage, 4u);
        Ariadne::mince(storage, 3u);
        Ariadne::recombine(storage);
        ARIADNE_TESTING_PROVE(storage.size() > 0u);

        // Snapshot the occupied location; it must retain its cells after the
        // original is cleared.
        const LabelledAugmentedPavingValue snapshot =
            make_labelled_augmented_paving(storage[q], system.continuous_state_space(q),
                                           system.continuous_auxiliary_space(q));
        const SizeType snapshot_size = snapshot.value().size();
        ARIADNE_TESTING_PROVE(snapshot_size > 0u);
        ARIADNE_TESTING_EXPECT_EQUAL(snapshot_size, storage[q].size());
        ARIADNE_TESTING_EXPECT_EQUAL(state_space(snapshot), system.continuous_state_space(q));
        ARIADNE_TESTING_EXPECT_EQUAL(auxiliary_space(snapshot), system.continuous_auxiliary_space(q));

        // The whole-family copy keeps every materialized location, its backend
        // and its auxiliary mapping; generic set operations do not mutate the original.
        const HybridAugmentedPavingValue family(storage);
        ARIADNE_TESTING_PROVE(family.size() == storage.size());
        ARIADNE_TESTING_PROVE(family.has_value(q));
        ARIADNE_TESTING_EXPECT_EQUAL(family[q].size(), storage[q].size());
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const RobddPaving*>(
                &static_cast<const PavingInterface&>(family[q].state_paving())) != nullptr);
        ARIADNE_TESTING_PROVE(Ariadne::subset(family, family));
        ARIADNE_TESTING_PROVE(Ariadne::intersect(family, family));
        HybridAugmentedPavingValue family_copy = family;
        Ariadne::remove(family_copy, family);
        ARIADNE_TESTING_PROVE(family_copy.has_value(q));
        ARIADNE_TESTING_PROVE(family_copy[q].is_empty());
        ARIADNE_TESTING_PROVE(not Ariadne::is_empty(storage));

        // Clear the location directly on the state paving.
        storage[q].state_paving().clear();
        ARIADNE_TESTING_PROVE(Ariadne::is_empty(storage));
        ARIADNE_TESTING_EXPECT_EQUAL(snapshot.value().size(), snapshot_size);
        ARIADNE_TESTING_PROVE(not snapshot.value().is_empty());
        ARIADNE_TESTING_PROVE(
            dynamic_cast<const RobddPaving*>(
                &static_cast<const PavingInterface&>(snapshot.value().state_paving())) != nullptr);

        // The generic value draws the flattened state-auxiliary pavings per location.
        Ariadne::adjoin_outer_approximation(storage, initial, 2u);
        ARIADNE_TESTING_PROVE(storage.size() > 0u);
        Set<DiscreteLocation> locations{q};
        FlattenedPavingCanvas selected_canvas;
        PavingDrawing::draw(selected_canvas, locations, Variables2d(x, y), storage);
        ARIADNE_TESTING_PROVE(selected_canvas.boundaries > 0u);
        FlattenedPavingCanvas all_canvas;
        PavingDrawing::draw(all_canvas, Set<DiscreteLocation>{}, Variables2d(x, y), storage);
        ARIADNE_TESTING_EXPECT_EQUAL(all_canvas.boundaries, selected_canvas.boundaries);
        FlattenedPavingCanvas excluded_canvas;
        PavingDrawing::draw(excluded_canvas, Set<DiscreteLocation>{DiscreteLocation(1)},
                            Variables2d(x, y), storage);
        ARIADNE_TESTING_EXPECT_EQUAL(excluded_canvas.boundaries, 0u);

        // The same projection drawn directly from the flattened state-auxiliary slice.
        FlattenedPavingCanvas direct_canvas;
        const BinaryTreePaving slice_paving = storage[q].state_auxiliary_paving();
        const RealSpace state_space = system.continuous_state_space(q);
        const Projection2d projection(slice_paving.grid().dimension(),
                                      state_space.index(x), state_space.index(y));
        PavingDrawing::draw(direct_canvas, projection, static_cast<const SubPavingInterface&>(slice_paving));
        ARIADNE_TESTING_EXPECT_EQUAL(direct_canvas.boundaries, selected_canvas.boundaries);

        ARIADNE_TESTING_NOTE(storage);

        storage[q].state_paving().clear();
        ARIADNE_TESTING_PROVE(Ariadne::is_empty(storage));
    }

    Void test_hybrid_augmented_paving_value_orbit() {
        std::function<Paving(const Grid&)> tree_backend =
            [](const Grid& selected) { return Paving(new BinaryTreePaving(selected)); };
        HybridAugmentedPavingValue initial = make_value(tree_backend);
        HybridBoundedConstraintSet initial_set(q, {0.9_dec <= x <= 1.1_dec, 0.9_dec <= y <= 1.1_dec});
        Ariadne::adjoin_outer_approximation(initial, initial_set, 2u);

        Orbit<HybridAugmentedPavingValue> orbit(initial);
        ARIADNE_TESTING_PROVE(orbit.initial().size() == initial.size());
        ARIADNE_TESTING_PROVE(orbit.reach().size() == 0u);
        ARIADNE_TESTING_PROVE(orbit.intermediate().size() == 0u);
        ARIADNE_TESTING_PROVE(orbit.final().size() == 0u);

        HybridAugmentedPavingValue reach = make_value(tree_backend);
        Ariadne::adjoin_outer_approximation(reach, initial_set, 2u);
        HybridAugmentedPavingValue intermediate = reach;
        HybridAugmentedPavingValue final_set = reach;

        Orbit<HybridAugmentedPavingValue> full_orbit(initial, reach, intermediate, final_set);
        ARIADNE_TESTING_PROVE(full_orbit.initial().size() == initial.size());
        ARIADNE_TESTING_EXPECT_EQUAL(full_orbit.reach()[q].size(), reach[q].size());
        ARIADNE_TESTING_EXPECT_EQUAL(full_orbit.intermediate()[q].size(), intermediate[q].size());
        ARIADNE_TESTING_EXPECT_EQUAL(full_orbit.final()[q].size(), final_set[q].size());
        ARIADNE_TESTING_PROVE(Ariadne::subset(full_orbit.initial(), full_orbit.reach()));

        // Location selection precedes drawing.
        FlattenedPavingCanvas canvas;
        PavingDrawing::draw(canvas, Set<DiscreteLocation>{q}, Variables2d(x, y), full_orbit.reach());
        ARIADNE_TESTING_PROVE(canvas.boundaries > 0u);
        FlattenedPavingCanvas all_locations_canvas;
        PavingDrawing::draw(all_locations_canvas, Set<DiscreteLocation>{}, Variables2d(x, y), full_orbit.reach());
        ARIADNE_TESTING_EXPECT_EQUAL(all_locations_canvas.boundaries, canvas.boundaries);
        FlattenedPavingCanvas excluded_canvas;
        PavingDrawing::draw(excluded_canvas, Set<DiscreteLocation>{DiscreteLocation(1)},
                            Variables2d(x, y), full_orbit.reach());
        ARIADNE_TESTING_EXPECT_EQUAL(excluded_canvas.boundaries, 0u);
    }

    Void test_owned_auxiliary_space_lifetime_and_identity() {
        auto mapping = [&]() {
            HybridAutomaton temporary("temporary");
            temporary.new_mode(q, {dot(x) = -x, dot(y) = -y});
            return make_hybrid_auxiliary_mapping(temporary);
        }();
        HybridSpace space = mapping.state_auxiliary_space();
        HybridSpace cloned(static_cast<const HybridSpaceInterface&>(space));
        ARIADNE_TESTING_PROVE(space.has_location(q));
        ARIADNE_TESTING_PROVE(not space.has_location(DiscreteLocation(1)));
        ARIADNE_TESTING_PROVE(space[q] == system.state_auxiliary_space()[q]);
        ARIADNE_TESTING_PROVE(definitely(space == cloned));
        auto independent = make_hybrid_auxiliary_mapping(system).state_auxiliary_space();
        ARIADNE_TESTING_PROVE(not definitely(space == independent));
        ARIADNE_TESTING_PROVE(not definitely(not(space == independent)));
        MonolithicHybridSpace ordinary;
        ordinary.new_location(q, space[q]);
        ARIADNE_TESTING_PROVE(not definitely(space == HybridSpace(ordinary)));
        std::ostringstream output;
        output << space;
        ARIADNE_TESTING_PROVE(output.str().find("temporary") != std::string::npos);
        ARIADNE_TESTING_PROVE(mapping(q).argument_space() == system.continuous_state_space(q));
    }
};

int main(int argc, char** argv) {
    DrawerManager::instance().set_drawer(BoxDrawer());

    ARIADNE_TESTING_SUITE("test_hybrid_augmented_paving");
    ARIADNE_TESTING_ADD_TEST(TestHybridAugmentedPaving, test_hybrid_augmented_paving_value_interface);
    ARIADNE_TESTING_ADD_TEST(TestHybridAugmentedPaving, test_hybrid_augmented_paving_value_orbit);
    ARIADNE_TESTING_ADD_TEST(TestHybridAugmentedPaving, test_owned_auxiliary_space_lifetime_and_identity);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

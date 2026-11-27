/***************************************************************************
 *            test_enclosure_augmented_paving.cpp
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

/** @file tests/test_enclosure_augmented_paving.cpp
 *  @brief Test enclosure augmented paving behavior and numerical invariants.
 */

// Verify the continuous and hybrid enclosure-to-augmented-paving adapters.
#include "function/taylor_function.hpp"
#include "logging/test_runner.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/paver.hpp"
#include "reasoning/enclosure_augmented_paving.hpp"
#include "reasoning/hybrid_augmented_paving_value.hpp"
#include "reasoning/hybrid_grid_value.hpp"
#include "reasoning/hybrid_paving_value.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestEnclosureAugmentedPaving : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_continuous() {
        ExactBoxType domain({
            {-1, 1},
            {-1, 1}
        });
        EnclosureConfiguration config(TaylorFunctionFactory(ThresholdSweeper<FloatDP>(dp, 1e-8)));
        Enclosure enclosure(domain, config);
        enclosure.set_auxiliary_mapping(EffectiveVectorMultivariateFunction::identity(2u));

        Grid grid(2u);
        Paver paver(new AffinePaver());
        AugmentedPaving generated = outer_approximation(enclosure, grid, 2u, paver);
        ARIADNE_TESTING_PROVE(generated.size() > 0u);
        ARIADNE_TESTING_EXPECT_EQUAL(generated.dimension(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(generated.state_auxiliary_paving().dimension(), 4u);

        AugmentedPaving explicit_paving(grid, enclosure.auxiliary_mapping());
        adjoin_outer_approximation_to(enclosure, explicit_paving, 2u, paver);
        ARIADNE_TESTING_PROVE(explicit_paving.size() > 0u);
        ARIADNE_TESTING_PROVE(subset(generated.state_paving(), explicit_paving.state_paving()));
        ARIADNE_TESTING_PROVE(subset(explicit_paving.state_paving(), generated.state_paving()));
        AugmentedPaving independent(grid, EffectiveVectorMultivariateFunction::identity(2u));
        adjoin_outer_approximation_to(enclosure, independent, 2u, paver);
        ARIADNE_TESTING_PROVE(subset(independent.state_paving(), generated.state_paving()));
        Enclosure without_auxiliary(domain, config);
        auto state_only = outer_approximation(without_auxiliary, grid, 2u, paver);
        ARIADNE_TESTING_PROVE(subset(state_only.state_paving(), generated.state_paving()));
    }

    Void test_hybrid() {
        DiscreteLocation location(1);
        RealVariable x("x"), y("y"), a("a");
        RealSpace state_space({x, y});
        RealSpace auxiliary_space({a});
        ExactBoxType domain({
            {-1, 1},
            {-1, 1}
        });
        EnclosureConfiguration config(TaylorFunctionFactory(ThresholdSweeper<FloatDP>(dp, 1e-8)));
        HybridEnclosure enclosure(HybridExactBox(location, state_space, domain), config);

        MonolithicHybridSpace state;
        state.new_location(location, state_space);
        const HybridSpace hybrid_space(state);
        auto unit_scalings = [&](const HybridSpace& space) {
            return Hybrid<Scalings>(space, [](const DiscreteLocation&, const RealSpace&) {
                return Scalings(1.0_x, Map<RealVariable, ExactDouble>{});
            });
        };
        auto backend = [](const Grid& g) { return Paving(new BinaryTreePaving(g)); };
        Hybrid<Paving> state_values = make_hybrid_pavings(
            hybrid_space, make_hybrid_grids(hybrid_space, unit_scalings(hybrid_space)), backend);
        MonolithicHybridSpace extended;
        extended.new_location(location, join(state_space, auxiliary_space));
        LabelledMapping labelled(state_space,
                                 EffectiveVectorMultivariateFunction({EffectiveScalarMultivariateFunction::coordinate(2u, 0u)}),
                                 auxiliary_space);
        HybridAuxiliaryMapping mapping(HybridSpace(extended),
                                       [labelled](const DiscreteLocation&) { return labelled; });
        HybridAugmentedPavingValue paving = make_hybrid_augmented_paving_value(state_values, mapping);
        Paver paver(new AffinePaver());
        adjoin_outer_approximation_to(enclosure, paving, 2u, paver);
        ARIADNE_TESTING_PROVE(paving.size() > 0u);
        ARIADNE_TESTING_PROVE(paving[location].size() > 0u);
        ARIADNE_TESTING_EXPECT_EQUAL(mapping(location).argument_space(), state_space);
        ARIADNE_TESTING_EXPECT_EQUAL(mapping(location).result_space(), auxiliary_space);

        // The same spatial precondition as the underlying HybridEnclosure API.
        MonolithicHybridSpace wrong_state;
        wrong_state.new_location(location, auxiliary_space);
        const HybridSpace wrong_hybrid_space(wrong_state);
        Hybrid<Paving> wrong_values = make_hybrid_pavings(
            wrong_hybrid_space, make_hybrid_grids(wrong_hybrid_space, unit_scalings(wrong_hybrid_space)), backend);
        HybridAugmentedPavingValue wrong = make_hybrid_augmented_paving_value(wrong_values, mapping);
        ARIADNE_TESTING_EXPECT_EXCEPTION(adjoin_outer_approximation_to(enclosure, wrong, 2u, paver));
    }

    Void test_hybrid_enclosure_paving_space_contract() {
        const DiscreteLocation q(1);
        const RealVariable x("x"), y("y");
        const RealSpace state_space({x, y});
        const ExactBoxType domain({
            {-1, 1},
            {-1, 1}
        });
        const EnclosureConfiguration config(TaylorFunctionFactory(ThresholdSweeper<FloatDP>(dp, 1e-8)));
        const HybridEnclosure initial(HybridExactBox(q, state_space, domain), config);

        auto unit_scalings = [](const HybridSpace& space) {
            return Hybrid<Scalings>(space, [](const DiscreteLocation&, const RealSpace&) {
                return Scalings(1.0_x, Map<RealVariable, ExactDouble>{});
            });
        };
        auto backend = [](const Grid& g) { return Paving(new BinaryTreePaving(g)); };

        MonolithicHybridSpace valid_space;
        valid_space.new_location(q, state_space);
        const HybridSpace valid_hybrid(valid_space);
        Hybrid<Paving> paving = make_hybrid_pavings(
            valid_hybrid, make_hybrid_grids(valid_hybrid, unit_scalings(valid_hybrid)), backend);
        const Paver paver(new AffinePaver());
        adjoin_outer_approximation_to(initial, paving, 0u, paver);
        ARIADNE_TESTING_PROVE(paving[q].size() > 0u);

        MonolithicHybridSpace incompatible;
        incompatible.new_location(q, RealSpace({y, x}));
        const HybridSpace incompatible_hybrid(incompatible);
        Hybrid<Paving> wrong = make_hybrid_pavings(
            incompatible_hybrid, make_hybrid_grids(incompatible_hybrid, unit_scalings(incompatible_hybrid)), backend);
        ARIADNE_TESTING_EXPECT_EXCEPTION(adjoin_outer_approximation_to(initial, wrong, 0u, paver));
    }

    Void test_hybrid_generic_value() {
        DiscreteLocation location(1);
        RealVariable x("x"), y("y"), a("a");
        RealSpace state_space({x, y});
        RealSpace auxiliary_space({a});
        ExactBoxType domain({
            {-1, 1},
            {-1, 1}
        });
        EnclosureConfiguration config(TaylorFunctionFactory(ThresholdSweeper<FloatDP>(dp, 1e-8)));
        HybridEnclosure enclosure(HybridExactBox(location, state_space, domain), config);

        MonolithicHybridSpace state;
        state.new_location(location, state_space);
        const HybridSpace hybrid_space(state);
        Paver paver(new AffinePaver());
        auto unit_scalings = [&](const HybridSpace& space) {
            return Hybrid<Scalings>(space, [](const DiscreteLocation&, const RealSpace&) {
                return Scalings(1.0_x, Map<RealVariable, ExactDouble>{});
            });
        };
        auto backend = [](const Grid& g) { return Paving(new BinaryTreePaving(g)); };
        Hybrid<Grid> grids = make_hybrid_grids(hybrid_space, unit_scalings(hybrid_space));

        LabelledMapping labelled(state_space,
                                 EffectiveVectorMultivariateFunction({EffectiveScalarMultivariateFunction::coordinate(2u, 0u)}),
                                 auxiliary_space);
        MonolithicHybridSpace extended;
        extended.new_location(location, join(state_space, auxiliary_space));
        HybridAuxiliaryMapping mapping(HybridSpace(extended),
                                       [labelled](const DiscreteLocation&) { return labelled; });

        // Independent regression: two identically built generic families must
        // receive identical cells from the same enclosure.
        HybridAugmentedPavingValue generic = make_hybrid_augmented_paving_value(
            make_hybrid_pavings(hybrid_space, grids, backend), mapping);
        adjoin_outer_approximation_to(enclosure, generic, 2u, paver);
        ARIADNE_TESTING_PROVE(generic.has_value(location));
        ARIADNE_TESTING_PROVE(generic[location].size() > 0u);
        HybridAugmentedPavingValue replica = make_hybrid_augmented_paving_value(
            make_hybrid_pavings(hybrid_space, grids, backend), mapping);
        adjoin_outer_approximation_to(enclosure, replica, 2u, paver);
        ARIADNE_TESTING_PROVE(subset(generic, replica));
        ARIADNE_TESTING_PROVE(subset(replica, generic));

        // The Hybrid<Paving> overload materializes the location through the
        // family's lazy factory and matches the augmented family's state paving.
        Hybrid<Paving> state_values = make_hybrid_pavings(hybrid_space, grids, backend);
        adjoin_outer_approximation_to(enclosure, state_values, 2u, paver);
        ARIADNE_TESTING_PROVE(state_values.has_value(location));
        ARIADNE_TESTING_EXPECT_EQUAL(state_values[location].size(), generic[location].state_paving().size());

        // The same spatial precondition holds on the generic family.
        MonolithicHybridSpace wrong_state;
        wrong_state.new_location(location, auxiliary_space);
        const HybridSpace wrong_hybrid_space(wrong_state);
        HybridAugmentedPavingValue wrong = make_hybrid_augmented_paving_value(
            make_hybrid_pavings(wrong_hybrid_space,
                                make_hybrid_grids(wrong_hybrid_space, unit_scalings(wrong_hybrid_space)), backend),
            mapping);
        ARIADNE_TESTING_EXPECT_EXCEPTION(adjoin_outer_approximation_to(enclosure, wrong, 2u, paver));
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_enclosure_augmented_paving");
    ARIADNE_TESTING_ADD_TEST(TestEnclosureAugmentedPaving, test_continuous);
    ARIADNE_TESTING_ADD_TEST(TestEnclosureAugmentedPaving, test_hybrid);
    ARIADNE_TESTING_ADD_TEST(TestEnclosureAugmentedPaving, test_hybrid_enclosure_paving_space_contract);
    ARIADNE_TESTING_ADD_TEST(TestEnclosureAugmentedPaving, test_hybrid_generic_value);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

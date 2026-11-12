/***************************************************************************
 *            test_analyser_contracts.cpp
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

/** @file tests/test_analyser_contracts.cpp
 *  @brief Test analyser contracts semantics and regressions.
 */

#include <memory>
#include <type_traits>

#include "paving/robdd_paving.hpp"
#include "reasoning/time_partition.hpp"

#include "ariadne-reasoning.hpp"
#include "ariadne-testing.hpp"

using namespace Ariadne;

template<class SYS>
struct ContractAnalyser : ReachabilityAnalyser<SYS> {
    using Base = ReachabilityAnalyser<SYS>;
    using Base::_adjoin_upper_reach_evolve;
    using Base::_checked_restriction;
    using Base::Base;
    auto storage() const {
        return this->_storage_factory(this->configuration().grid(), this->system());
    }
};

class TestAnalyserContracts : public Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    template<class SYS, class SET>
    void check_safety_escape(
        ContractAnalyser<SYS>& analyser, SET const& initial, SET const& safe) {
        analyser.configuration().set_maximum_grid_fineness(3u);
        analyser.configuration().set_maximum_grid_extent(4u);
        auto certificate = analyser.verify_safety(initial, safe);
        auto initial_cells = analyser.storage();
        initial_cells.adjoin_outer_approximation(initial, 3u);
        ARIADNE_TESTING_PROVE(subset(initial_cells, certificate.safe_set));
        ARIADNE_TESTING_PROVE(is_indeterminate(certificate.is_safe));
        ARIADNE_TESTING_PROVE(not subset(certificate.chain_reach_set, certificate.safe_set));
    }

    Void test_safety_becomes_unknown_after_initial_containment() {
        RealVariable x("x");
        BoundedConstraintSet initial(RealBox({
            {-1, -0.75_dec}
        }));
        BoundedConstraintSet safe(RealBox({
            {-2, 0}
        }));
        VectorField system(List<DottedRealAssignment>{dot(x) = 1});
        GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-3_pr));
        VectorFieldEvolver evolver(system, integrator);
        ContractAnalyser<VectorField> continuous(evolver);
        continuous.configuration().set_bounding_domain(ContractAnalyser<VectorField>::BoundingDomainType({
            {-4, 4}
        }));
        check_safety_escape(continuous, initial, safe);
        IteratedMap map(List<PrimedRealAssignment>{next(x) = x + 1});
        IteratedMapEvolver map_evolver(map);
        ContractAnalyser<IteratedMap> discrete(map_evolver);
        discrete.configuration().set_bounding_domain(ContractAnalyser<IteratedMap>::BoundingDomainType({
            {-4, 4}
        }));
        check_safety_escape(discrete, initial, safe);

        DiscreteLocation q;
        HybridAutomaton hybrid;
        hybrid.new_mode(q, {dot(x) = 1});
        GeneralHybridEvolver hybrid_evolver(hybrid);
        ContractAnalyser<HybridAutomatonInterface> analyser(hybrid_evolver);
        HybridExactBoxes bounding;
        bounding.insert(q, hybrid.continuous_state_space(q), ExactBoxType({
                                                                 {-4, 4}
        }));
        analyser.configuration().set_bounding_domain(bounding);
        check_safety_escape(analyser, HybridBoundedConstraintSet(q, {-1 <= x <= -0.75_dec}),
                            HybridBoundedConstraintSet(q, {-2 <= x <= 0}));
    }

    Void test_whole_steps_never_round_past_target() {
        for (auto target: {0_q, 1 / 16_q, 3 / 16_q, 1 / 4_q, 5 / 16_q, 7 / 16_q}) {
            auto steps = ReasoningDetail::whole_time_steps(Real(target), Real(1 / 4_q));
            Rational consumed = Rational(steps) * Rational(1 / 4_q);
            ARIADNE_TESTING_PROVE(consumed <= target);
            ARIADNE_TESTING_PROVE(target - consumed < 1 / 4_q);
        }
        ARIADNE_TESTING_EXPECT_EQUAL(ReasoningDetail::whole_time_steps(Real(3 / 16_q), Real(1 / 4_q)), Natural(0u));
        ARIADNE_TESTING_EXPECT_EQUAL(ReasoningDetail::whole_time_steps(Real(7 / 16_q), Real(1 / 4_q)), Natural(1u));
    }

    template<class SYS, class SET>
    Void check_finite_times(
        ContractAnalyser<SYS>& analyser, SET const& initial, SET const& empty,
        SET const& far_safe, typename ContractAnalyser<SYS>::TimeType partial,
        typename ContractAnalyser<SYS>::TimeType exact) {
        using AnalyserTime = typename ContractAnalyser<SYS>::TimeType;
        analyser.configuration().set_maximum_grid_fineness(0u);
        if constexpr (std::is_same_v<SYS, HybridAutomatonInterface>)
            analyser.configuration().set_lock_to_grid_time(exact.continuous_time().get_d());
        else
            analyser.configuration().set_lock_to_grid_time(exact);
        for (const AnalyserTime& time: {AnalyserTime(0), partial, exact}) {
            ARIADNE_TESTING_PROVE(not analyser.upper_evolve(initial, time).is_empty());
            ARIADNE_TESTING_PROVE(not analyser.upper_reach(initial, time).is_empty());
            auto pair = analyser.upper_reach_evolve(initial, time);
            ARIADNE_TESTING_PROVE(not pair.first.is_empty());
            ARIADNE_TESTING_PROVE(not pair.second.is_empty());
            ARIADNE_TESTING_PROVE(analyser.upper_evolve(empty, time).is_empty());
            ARIADNE_TESTING_PROVE(analyser.upper_reach(empty, time).is_empty());
            auto empty_pair = analyser.upper_reach_evolve(empty, time);
            ARIADNE_TESTING_PROVE(empty_pair.first.is_empty());
            ARIADNE_TESTING_PROVE(empty_pair.second.is_empty());
        }
        analyser.configuration().set_bounding_domain_ptr(nullptr);
        auto certificate = analyser.verify_safety(initial, far_safe);
        ARIADNE_TESTING_PROVE(not definitely(certificate.is_safe));
        ARIADNE_TESTING_PROVE(not certificate.chain_reach_set.is_empty());
        ARIADNE_TESTING_PROVE(definitely(analyser.verify_safety(empty, far_safe).is_safe));
    }

    Void test_continuous_and_discrete_time_boundaries() {
        RealVariable x("x");
        BoundedConstraintSet initial(RealBox({
            {0.25_dec, 0.5_dec}
        }));
        auto coordinate = EffectiveScalarMultivariateFunction::coordinate(1u, 0u);
        BoundedConstraintSet empty(RealBox({
                                       {0, 1}
        }),
                                   {coordinate >= 2});
        BoundedConstraintSet far_safe(RealBox({
            {4, 5}
        }));
        VectorField flow(List<DottedRealAssignment>{dot(x) = 0});
        GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-3_pr));
        VectorFieldEvolver flow_evolver(flow, integrator);
        ContractAnalyser<VectorField> continuous(flow_evolver);
        check_finite_times(continuous, initial, empty, far_safe, Real(1 / 8_q), Real(1 / 4_q));
        continuous.configuration().set_transient_time(Real(1 / 8_q));
        continuous.configuration().set_maximum_grid_extent(0u);
        continuous.configuration().set_outer_overspill_policy(ChainOverspillPolicy::IGNORE);
        ARIADNE_TESTING_PROVE(not continuous.lower_reach(initial).is_empty());
        ARIADNE_TESTING_PROVE(not continuous.outer_chain_reach(initial).is_empty());

        IteratedMap map(List<PrimedRealAssignment>{next(x) = x / 2});
        IteratedMapEvolver map_evolver(map);
        ContractAnalyser<IteratedMap> discrete(map_evolver);
        check_finite_times(discrete, initial, empty, far_safe, Integer(1), Integer(2));
    }

    Void test_hybrid_time_boundaries() {
        RealVariable x("x");
        DiscreteLocation q;
        HybridAutomaton system;
        system.new_mode(q, {dot(x) = 0});
        GeneralHybridEvolver evolver(system);
        ContractAnalyser<HybridAutomatonInterface> analyser(evolver);
        HybridBoundedConstraintSet initial(q, {0.25_dec <= x <= 0.5_dec});
        HybridBoundedConstraintSet empty(q, {0 <= x <= 1}, {x >= 2});
        HybridBoundedConstraintSet far_safe(q, {4 <= x <= 5});
        check_finite_times(analyser, initial, empty, far_safe, HybridTime(Real(1 / 8_q), Integer(1)), HybridTime(Real(1 / 4_q), Integer(1)));
    }

    template<class SYS>
    Void check(ContractAnalyser<SYS>& analyser) {
        auto manager = std::make_shared<RobddManager>();
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument, analyser.set_paving_factory({}));
        analyser.set_paving_factory([manager](const Grid& grid) {
            return Paving(new RobddPaving(manager, grid));
        });
        auto input = analyser.storage();
        auto bounding = analyser.storage();
        auto empty = analyser.storage();
        auto& input_state = input.state_paving();
        auto& bounding_state = bounding.state_paving();
        Grid grid(1u);
        GridCell root(grid, 0u, BinaryWord{});
        if constexpr (std::is_same_v<SYS, HybridAutomatonInterface>) {
            DiscreteLocation q;
            input_state.adjoin(HybridGridCell(q, analyser.system().continuous_state_space(q), root));
            bounding_state.adjoin(HybridGridCell(q, analyser.system().continuous_state_space(q), root.split(false)));
        } else {
            input_state.adjoin(root);
            bounding_state.adjoin(root.split(false));
        }
        for (auto policy: {ChainOverspillPolicy::IGNORE, ChainOverspillPolicy::WARNING,
                           ChainOverspillPolicy::ERROR}) {
            auto unchanged = input;
            analyser._checked_restriction(unchanged, empty, policy);
            ARIADNE_TESTING_PROVE(subset(input, unchanged));
            auto restricted = input;
            if (policy == ChainOverspillPolicy::ERROR) {
                ARIADNE_TESTING_EXPECT_EXCEPTION(analyser._checked_restriction(restricted, bounding, policy));
            } else {
                analyser._checked_restriction(restricted, bounding, policy);
            }
            ARIADNE_TESTING_PROVE(subset(restricted, bounding));
            ARIADNE_TESTING_PROVE(subset(bounding, restricted));
            ARIADNE_TESTING_PROVE(not subset(input, restricted));
        }
    }

    Void test_continuous_and_discrete_restriction() {
        RealVariable x("x");
        VectorField flow(List<DottedRealAssignment>{dot(x) = -x});
        GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-3_pr));
        VectorFieldEvolver flow_evolver(flow, integrator);
        ContractAnalyser<VectorField> continuous(flow_evolver);
        check(continuous);
        IteratedMap map(List<PrimedRealAssignment>{next(x) = x / 2});
        IteratedMapEvolver map_evolver(map);
        ContractAnalyser<IteratedMap> discrete(map_evolver);
        check(discrete);
    }

    Void test_hybrid_restriction() {
        RealVariable x("x");
        DiscreteLocation q;
        HybridAutomaton system;
        system.new_mode(q, {dot(x) = -x});
        GeneralHybridEvolver evolver(system);
        ContractAnalyser<HybridAutomatonInterface> analyser(evolver);
        check(analyser);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_analyser_contracts");
    ARIADNE_TESTING_ADD_TEST(TestAnalyserContracts, test_whole_steps_never_round_past_target);
    ARIADNE_TESTING_ADD_TEST(TestAnalyserContracts, test_safety_becomes_unknown_after_initial_containment);
    ARIADNE_TESTING_ADD_TEST(TestAnalyserContracts, test_continuous_and_discrete_restriction);
    ARIADNE_TESTING_ADD_TEST(TestAnalyserContracts, test_hybrid_restriction);
    ARIADNE_TESTING_ADD_TEST(TestAnalyserContracts, test_continuous_and_discrete_time_boundaries);
    ARIADNE_TESTING_ADD_TEST(TestAnalyserContracts, test_hybrid_time_boundaries);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}

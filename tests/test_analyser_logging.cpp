/***************************************************************************
 *            test_analyser_logging.cpp
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

/** @file tests/test_analyser_logging.cpp
 *  @brief Test analyser logging semantics and regressions.
 */

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <type_traits>

#include "threading/thread_manager.hpp"

#include "ariadne-reasoning.hpp"
#include "ariadne-testing.hpp"

using namespace Ariadne;

template<class SYS>
struct LoggingAnalyser : ReachabilityAnalyser<SYS> {
    using Base = ReachabilityAnalyser<SYS>;
    using Base::_adjoin_upper_reach_evolve;
    using Base::_upper_reach;
    using Base::Base;
    auto storage() const {
        return this->_storage_factory(this->configuration().grid(), this->system());
    }
};

class TestAnalyserLogging : public Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    template<class SYS, class SET>
    void check(LoggingAnalyser<SYS>& analyser,
               SET const& initial, SET const& safe,
               typename LoggingAnalyser<SYS>::TimeType horizon) {
        auto& config = analyser.configuration();
        config.set_maximum_grid_extent(0u);
        config.set_maximum_grid_fineness(0u);
        config.set_outer_overspill_policy(ChainOverspillPolicy::IGNORE);
        if constexpr (std::is_same_v<SYS, HybridAutomatonInterface>) {
            config.set_lock_to_grid_time(horizon.continuous_time().get_d());
            config.set_transient_time(horizon.continuous_time().get_d());
        } else {
            config.set_lock_to_grid_time(horizon);
            config.set_transient_time(horizon);
        }
        auto lower_evolved = analyser.lower_evolve(initial, horizon);
        auto lower_reached = analyser.lower_reach(initial, horizon);
        auto lower_pair = analyser.lower_reach_evolve(initial, horizon);
        ARIADNE_TESTING_PROVE(not lower_evolved.is_empty());
        ARIADNE_TESTING_PROVE(not lower_reached.is_empty());
        ARIADNE_TESTING_PROVE(not lower_pair.first.is_empty());
        ARIADNE_TESTING_PROVE(not analyser.lower_reach(initial).is_empty());
        ARIADNE_TESTING_PROVE(not analyser.upper_evolve(initial, horizon).is_empty());
        auto reached = analyser.upper_reach(initial, horizon);
        ARIADNE_TESTING_PROVE(not reached.is_empty());
        auto upper_pair = analyser.upper_reach_evolve(initial, horizon);
        ARIADNE_TESTING_PROVE(not upper_pair.second.is_empty());
        // Repeat with a positive remainder while diagnostics are enabled.
        if constexpr (std::is_same_v<SYS, HybridAutomatonInterface>)
            config.set_lock_to_grid_time(2.0 * horizon.continuous_time().get_d());
        else
            config.set_lock_to_grid_time(Natural(2u) * horizon);
        ARIADNE_TESTING_PROVE(not analyser.upper_evolve(initial, horizon).is_empty());
        ARIADNE_TESTING_PROVE(not analyser.upper_reach(initial, horizon).is_empty());
        auto remainder_pair = analyser.upper_reach_evolve(initial, horizon);
        ARIADNE_TESTING_PROVE(not remainder_pair.second.is_empty());
        if constexpr (std::is_same_v<SYS, HybridAutomatonInterface>)
            config.set_lock_to_grid_time(horizon.continuous_time().get_d());
        else
            config.set_lock_to_grid_time(horizon);
        ARIADNE_TESTING_PROVE(not analyser._upper_reach(reached, horizon, 0u, analyser.evolver()).is_empty());
        ARIADNE_TESTING_PROVE(not analyser.outer_chain_reach(initial).is_empty());
        config.set_maximum_grid_extent(4u);
        ARIADNE_TESTING_PROVE(definitely(analyser.verify_safety(initial, safe).is_safe));

        // Exercise progress diagnostics in both scheduler arms, including
        // the lane-local orbit messages and final publication summaries.
        auto input = analyser.storage();
        if constexpr (std::is_same_v<SYS, HybridAutomatonInterface>) {
            DiscreteLocation q;
            GridCell root(input.grid()[q], 0u, BinaryWord{});
            input.state_paving().adjoin(HybridGridCell(q,
                                                       analyser.system().continuous_state_space(q), root));
        } else
            input.state_paving().adjoin(GridCell(Grid(1u), 0u, BinaryWord{}));
        auto& threads = ThreadManager::instance();
        auto previous = threads.concurrency();
        struct Restore {
            ThreadManager& manager;
            std::size_t saved;
            ~Restore() { manager.set_concurrency(saved); }
        } restore{threads, previous};
        for (std::size_t workers: std::initializer_list<std::size_t>{0u, std::min<std::size_t>(2u, threads.maximum_concurrency())}) {
            threads.set_concurrency(workers);
            auto reach = analyser.storage(), evolved = analyser.storage();
            analyser._adjoin_upper_reach_evolve(reach, evolved, input, horizon, 2u, analyser.evolver());
            ARIADNE_TESTING_PROVE(not reach.is_empty());
            ARIADNE_TESTING_PROVE(not evolved.is_empty());
        }
    }

    void test_continuous_and_discrete_diagnostics() {
        RealVariable x("x");
        BoundedConstraintSet initial(RealBox({
            {0.25_dec, 0.5_dec}
        }));
        BoundedConstraintSet safe(RealBox({
            {-4, 4}
        }));
        VectorField system(List<DottedRealAssignment>{dot(x) = -x});
        GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-3_pr));
        VectorFieldEvolver evolver(system, integrator);
        LoggingAnalyser<VectorField> continuous(evolver);
        check(continuous, initial, safe, Real(1 / 16_q));
        IteratedMap map(List<PrimedRealAssignment>{next(x) = x / 2});
        IteratedMapEvolver map_evolver(map);
        LoggingAnalyser<IteratedMap> discrete(map_evolver);
        check(discrete, initial, safe, Integer(1));
    }

    void test_hybrid_diagnostics() {
        RealVariable x("x");
        DiscreteLocation q;
        HybridAutomaton system;
        system.new_mode(q, {dot(x) = -x});
        GeneralHybridEvolver evolver(system);
        LoggingAnalyser<HybridAutomatonInterface> analyser(evolver);
        check(analyser, HybridBoundedConstraintSet(q, {0.25_dec <= x <= 0.5_dec}),
              HybridBoundedConstraintSet(q, {-4 <= x <= 4}), HybridTime(Real(1 / 16_q), Integer(1)));
    }
};

int main(int argc, char** argv) {
    Logger::instance().redirect_to_file("test_analyser_logging.log");
    Logger::instance().configuration().set_verbosity(6u);
    ARIADNE_TESTING_SUITE("test_analyser_logging");
    // CTest executes the continuous/discrete and hybrid fixture cases in
    // separate processes, retaining their full assertions and TSAN coverage.
    const char* selected_case = std::getenv("ARIADNE_LOGGING_TEST_CASE");
    const std::string_view selection = selected_case != nullptr ? selected_case : "";
    if (selection != "" && selection != "continuous" && selection != "hybrid") {
        std::cerr << "Unknown ARIADNE_LOGGING_TEST_CASE: " << selection << '\n';
        return 2;
    }
    if (selection.empty() || selection == "continuous")
        ARIADNE_TESTING_ADD_TEST(TestAnalyserLogging, test_continuous_and_discrete_diagnostics);
    if (selection.empty() || selection == "hybrid")
        ARIADNE_TESTING_ADD_TEST(TestAnalyserLogging, test_hybrid_diagnostics);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}

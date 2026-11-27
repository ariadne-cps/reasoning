/***************************************************************************
 *            analyser_logging_common.hpp
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

/** @file tests/analyser_logging_common.hpp
 *  @brief Shared analyser logging test helpers.
 */

#ifndef ARIADNE_TEST_ANALYSER_LOGGING_COMMON_HPP
#define ARIADNE_TEST_ANALYSER_LOGGING_COMMON_HPP

#include <algorithm>
#include <type_traits>

#include "threading/thread_manager.hpp"

#include "ariadne-reasoning.hpp"
#include "ariadne-testing.hpp"

using namespace Ariadne;

// Free is_empty for the generic hybrid value storage, member call for legacy storage.
template<class SYS>
Bool storage_is_empty(const typename ReachabilityAnalyser<SYS>::StorageType& storage) {
    if constexpr (std::is_same_v<SYS, HybridAutomatonInterface>)
        return is_empty(storage);
    else
        return storage.is_empty();
}

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

class TestAnalyserLoggingBase : public Testing::ContextFixture {
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
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(lower_evolved));
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(lower_reached));
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(lower_pair.first));
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(analyser.lower_reach(initial)));
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(analyser.upper_evolve(initial, horizon)));
        auto reached = analyser.upper_reach(initial, horizon);
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(reached));
        auto upper_pair = analyser.upper_reach_evolve(initial, horizon);
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(upper_pair.second));
        // Repeat with a positive remainder while diagnostics are enabled.
        if constexpr (std::is_same_v<SYS, HybridAutomatonInterface>)
            config.set_lock_to_grid_time(2.0 * horizon.continuous_time().get_d());
        else
            config.set_lock_to_grid_time(Natural(2u) * horizon);
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(analyser.upper_evolve(initial, horizon)));
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(analyser.upper_reach(initial, horizon)));
        auto remainder_pair = analyser.upper_reach_evolve(initial, horizon);
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(remainder_pair.second));
        if constexpr (std::is_same_v<SYS, HybridAutomatonInterface>)
            config.set_lock_to_grid_time(horizon.continuous_time().get_d());
        else
            config.set_lock_to_grid_time(horizon);
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(analyser._upper_reach(reached, horizon, 0u, analyser.evolver())));
        ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(analyser.outer_chain_reach(initial)));
        config.set_maximum_grid_extent(4u);
        ARIADNE_TESTING_PROVE(definitely(analyser.verify_safety(initial, safe).is_safe));

        // Exercise progress diagnostics in both scheduler arms, including
        // the lane-local orbit messages and final publication summaries.
        auto input = analyser.storage();
        if constexpr (std::is_same_v<SYS, HybridAutomatonInterface>) {
            DiscreteLocation q;
            Hybrid<Grid> materialized_grids = analyser.configuration().grid();
            GridCell root(materialized_grids[q], 0u, BinaryWord{});
            input[q].state_paving().adjoin(root);
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
            ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(reach));
            ARIADNE_TESTING_PROVE(not storage_is_empty<SYS>(evolved));
        }
    }
};

#endif // ARIADNE_TEST_ANALYSER_LOGGING_COMMON_HPP

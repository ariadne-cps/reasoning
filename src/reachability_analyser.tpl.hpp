/***************************************************************************
 *            src/reachability_analyser.tpl.hpp
 *
 *  Copyright  2006-20  Alberto Casagrande, Pieter Collins, Davide Bresolin
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

/** @file src/reachability_analyser.tpl.hpp
 *  @brief Template implementations of reachability analysis algorithms.
 */

#include <algorithm>
#include <cfenv>
#include <exception>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <valarray>
#include <vector>

#include "algebra/matrix.hpp"
#include "algebra/vector.hpp"
#include "analysis_paving_factory.hpp"
#include "function/functional.hpp"
#include "geometry/box.hpp"
#include "integration/integrator.hpp"
#include "logging/logging.hpp"
#include "logging/progress_indicator.hpp"
#include "numeric/numeric.hpp"
#include "parallel_reachability.hpp"
#include "paving/binary_tree_paving.hpp"
#include "reasoning/enclosure_augmented_paving.hpp"
#include "reasoning/reachability_analyser.hpp"
#include "reasoning/time_partition.hpp"
#include "rendering/figure.hpp"
#include "set/function_set.hpp"
#include "set/list_set.hpp"
#include "threading/floating_point_environment.hpp"
#include "threading/task_group.hpp"
#include "threading/thread_manager.hpp"
#include "utility/exceptions.hpp"

#include "equation-solving/equation_solver.hpp"
#include "linear-programming/linear_programming.hpp"

namespace Ariadne {

class IteratedMap;

OutputStream& operator<<(OutputStream& os, const ChainOverspillPolicy& policy);

namespace ReasoningDetail {

// Storage operations are members on the legacy labelled pavings and free
// functions on the generic hybrid values; dispatch on the storage type.
template<class STORAGE, class SET>
Void storage_adjoin_lower_approximation(STORAGE& storage, const SET& set, Nat extent, Nat fineness) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>)
        adjoin_lower_approximation(storage, set, extent, fineness);
    else
        storage.adjoin_lower_approximation(set, extent, fineness);
}

template<class STORAGE, class SET>
Void storage_adjoin_outer_approximation(STORAGE& storage, const SET& set, Nat fineness) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>)
        adjoin_outer_approximation(storage, set, fineness);
    else
        storage.adjoin_outer_approximation(set, fineness);
}

template<class STORAGE>
Void storage_adjoin(STORAGE& left, const STORAGE& right) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>)
        adjoin(left, right);
    else
        left.adjoin(right);
}

template<class STORAGE>
Void storage_remove(STORAGE& left, const STORAGE& right) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>)
        remove(left, right);
    else
        left.remove(right);
}

template<class STORAGE>
Void storage_restrict(STORAGE& left, const STORAGE& right) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>)
        restrict(left, right);
    else
        left.restrict(right);
}

template<class STORAGE>
Void storage_restrict_to_extent(STORAGE& storage, Nat extent) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>)
        restrict_to_extent(storage, extent);
    else
        storage.restrict_to_extent(extent);
}

template<class STORAGE>
Void storage_mince(STORAGE& storage, Nat fineness) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>)
        mince(storage, fineness);
    else
        storage.mince(fineness);
}

template<class STORAGE>
Void storage_recombine(STORAGE& storage) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>)
        recombine(storage);
    else
        storage.recombine();
}

template<class STORAGE>
Bool storage_is_empty(const STORAGE& storage) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>)
        return is_empty(storage);
    else
        return storage.is_empty();
}

// Grid cells of a storage object, as the boxes accepted by the evolver.
// Hybrid values iterate the cells of their per-location state pavings.
template<class STORAGE>
auto first_storage_cell_box(const STORAGE& cells) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>) {
        auto loc = cells.begin();
        auto cell = loc->second.state_paving().begin();
        return HybridExactBox(loc->first, cells.space()[loc->first], cell->box());
    } else {
        return cells.begin()->box();
    }
}

template<class STORAGE>
using StorageCellBox = std::decay_t<decltype(first_storage_cell_box(std::declval<const STORAGE&>()))>;

template<class STORAGE, class FN>
Void for_each_storage_cell(const STORAGE& cells, FN&& fn) {
    if constexpr (std::is_same_v<STORAGE, HybridAugmentedPavingValue>) {
        for (auto iter = cells.begin(); iter != cells.end(); ++iter) {
            const DiscreteLocation location = iter->first;
            const RealSpace space = cells.space()[location];
            for (const auto& cell: iter->second.state_paving())
                fn(HybridExactBox(location, space, cell.box()));
        }
    } else {
        for (const auto& cell: cells)
            fn(cell.box());
    }
}

} // namespace ReasoningDetail

template<class SYS>
ReachabilityAnalyser<SYS>::
    ReachabilityAnalyser(const EvolverType& evolver)
    : _system(evolver.system().clone()), _evolver(evolver.clone()), _configuration(new ConfigurationType(*this)), _storage_factory([](const GridType& grid, const SystemType& system) -> StorageType {
          return make_analysis_paving<StorageType>(grid, system);
      }) {
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    clone() const
    -> ReachabilityAnalyser<SYS>* {
    return new ReachabilityAnalyser(*this);
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    _reach_evolve_resume(const ListSet<EnclosureType>& initial_enclosures,
                         const TimeType& time,
                         const Nat accuracy,
                         ListSet<EnclosureType>& evolve_enclosures,
                         Semantics semantics,
                         const EvolverType& evolver) const
    -> Pair<StorageType, StorageType> {
    LOGGING_SCOPE_CREATE;
    const GridType& grid = this->_configuration->grid();
    Pair<StorageType, StorageType> result = make_pair(this->_storage_factory(grid, this->system()), this->_storage_factory(grid, this->system()));
    StorageType& reach_cells = result.first;
    StorageType& evolve_cells = result.second;

    for (auto initial_enclosure: initial_enclosures) {
        auto orbit = evolver.orbit(initial_enclosure, time, semantics);

        for (auto enclosure: orbit.reach())
            adjoin_outer_approximation_to(enclosure, reach_cells, accuracy, this->_configuration->paver());
        LOGGING_PRINTLN_AT(1, "final reach size = " << reach_cells.size());

        for (auto enclosure: orbit.final()) {
            LOGGING_PRINTLN_AT(2, "enclosure = " << enclosure);
            adjoin_outer_approximation_to(enclosure, evolve_cells, accuracy, this->_configuration->paver());
            EnclosureType new_enclosure = enclosure;
            new_enclosure.clear_time();
            evolve_enclosures.adjoin(new_enclosure);
        }
    }
    LOGGING_PRINTLN("final evolve size = " << evolve_cells.size());
    LOGGING_PRINTLN("final evolve enclosures size = " << evolve_enclosures.size());
    return result;
}

// Helper functions for operators on lists of sets.
template<class SYS>
auto ReachabilityAnalyser<SYS>::
    _upper_reach(const StorageType& set,
                 const TimeType& time,
                 const Nat accuracy,
                 const EvolverType& evolver) const
    -> StorageType {
    const GridType& grid = this->_configuration->grid();
    StorageType result = this->_storage_factory(grid, this->system());
    StorageType cells = set;
    ReasoningDetail::storage_mince(cells, accuracy);
    ReasoningDetail::for_each_storage_cell(cells, [&](const auto& box) {
        EnclosureType initial_enclosure = evolver.enclosure(box);
        ListSet<EnclosureType> reach = evolver.orbit(initial_enclosure, time, Semantics::UPPER).reach();
        for (auto enclosure: reach) {
            adjoin_outer_approximation_to(enclosure, result, accuracy, this->_configuration->paver());
        }
    });
    return result;
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    _upper_evolve(const StorageType& set,
                  const TimeType& time,
                  const Nat accuracy,
                  const EvolverType& evolver) const
    -> StorageType {
    LOGGING_SCOPE_CREATE;
    const GridType& grid = this->_configuration->grid();
    StorageType result = this->_storage_factory(grid, this->system());
    StorageType cells = set;
    ReasoningDetail::storage_mince(cells, accuracy);
    ReasoningDetail::for_each_storage_cell(cells, [&](const auto& box) {
        LOGGING_PRINTLN_AT(1, "Evolving cell = " << box);
        EnclosureType initial_enclosure = evolver.enclosure(box);
        ListSet<EnclosureType> final = evolver.orbit(initial_enclosure, time, Semantics::UPPER).final();
        for (auto enclosure: final)
            adjoin_outer_approximation_to(enclosure, result, accuracy, this->_configuration->paver());
    });
    LOGGING_PRINTLN("_upper_evolve result size = " << result.size());
    return result;
}

template<class SYS>
Void ReachabilityAnalyser<SYS>::
    _adjoin_upper_reach_evolve(StorageType& reach_cells,
                               StorageType& evolve_cells,
                               const StorageType& set,
                               const TimeType& time,
                               const Nat accuracy,
                               const EvolverType& evolver) const {
    LOGGING_SCOPE_CREATE;
    StorageType cells = set;
    ReasoningDetail::storage_mince(cells, accuracy);

    using BoxType = ReasoningDetail::StorageCellBox<StorageType>;
    std::vector<BoxType> work;
    ReasoningDetail::for_each_storage_cell(cells, [&](const BoxType& box) { work.push_back(box); });

    LOGGING_PRINTLN("Evolving " << work.size() << " cells");
    ProgressIndicator indicator(static_cast<double>(work.size()));
    LOGGING_SCOPE_PRINTHOLD("[" << indicator.symbol() << "] "
                                << indicator.percentage() << "% of cells evolved.");

    // Continuous pavings own a stable Grid and synchronise their internal
    // mutations. Hybrid pavings additionally have a location map whose
    // lookup/insertion sequence must be serialised as a single operation.
    // Orbit evolution remains concurrent in both cases.
    std::mutex hybrid_adjoin_mutex;
    auto adjoin_incrementally = [&](const auto& enclosure, StorageType& target) {
        if constexpr (std::is_same_v<StorageType, HybridAugmentedPavingValue>) {
            std::lock_guard<std::mutex> lock(hybrid_adjoin_mutex);
            adjoin_outer_approximation_to(enclosure, target, accuracy, this->_configuration->paver());
        } else {
            adjoin_outer_approximation_to(enclosure, target, accuracy, this->_configuration->paver());
        }
    };

    // A single progress line belongs to the analyser scope, not to the
    // individual worker scopes. Serialise updates and output together so
    // every completed cell advances the global indicator.
    std::mutex progress_mutex;
    const std::string progress_scope = logscopemanager.scope();
    auto report_cell_completed = [&] {
        std::lock_guard<std::mutex> lock(progress_mutex);
        indicator.update_current(indicator.current_value() + 1.0);
        if (not Logger::instance().is_muted_at(0)) {
            std::ostringstream message;
            message << "[" << indicator.symbol() << "] "
                    << indicator.percentage() << "% of cells evolved.";
            Logger::instance().hold(progress_scope, message.str());
        }
    };

    auto process_cell = [&](const BoxType& box, const EvolverType& local_evolver) {
        LOGGING_PRINTLN_AT(1, "evolving cell = " << box);
        EnclosureType initial_enclosure = local_evolver.enclosure(box);
        auto orbit = local_evolver.orbit(initial_enclosure, time, Semantics::UPPER);
        LOGGING_PRINTLN_AT(1, "computed " << orbit.reach().size() << " reach enclosures and " << orbit.final().size() << " final enclosures.");

        // Preserve the existing covered-region pruning with incremental writes.
        for (auto enclosure: orbit.reach())
            adjoin_incrementally(enclosure, reach_cells);
        for (auto enclosure: orbit.final())
            adjoin_incrementally(enclosure, evolve_cells);
        report_cell_completed();
    };

    bool ran_parallel = false;
    // The same bounded cell-level scheduler serves continuous and hybrid
    // template instantiations. Only hybrid publication needs a map lock.
    {
        ThreadManager& threads = ThreadManager::instance();
        if (threads.concurrency() >= 2u and work.size() >= 2u) {
            // Detached input snapshot, even if set aliases a destination.
            // The cell boxes are already materialised in work.
            const std::size_t lanes = std::min<std::size_t>(
                {threads.concurrency(), work.size(), 8u});
            const int rounding = ReasoningDetail::require_caller_rounding_mode(std::fegetround());

            // The evolver is cloned per lane. Continuous clones retain
            // read-only integrator/configuration handles; hybrid evolvers
            // may supply deeper clones. No reconfiguration during a call.
            std::vector<std::unique_ptr<EvolverType>> local_evolvers;
            local_evolvers.reserve(lanes);
            for (std::size_t i = 0u; i < lanes; ++i)
                local_evolvers.emplace_back(evolver.clone());

            auto process_lane = [&](std::size_t lane) {
                ScopedRoundingMode restore_rounding(
                    std::optional<int>{rounding});
                const std::size_t base = work.size() / lanes;
                const std::size_t extra = work.size() % lanes;
                const std::size_t first = lane * base + std::min(lane, extra);
                const std::size_t last = first + base + (lane < extra ? 1u : 0u);
                for (std::size_t i = first; i < last; ++i)
                    process_cell(work[i], *local_evolvers[lane]);
            };

            ReasoningDetail::run_reachability_lanes(lanes, process_lane, threads);
            ran_parallel = true;
        }
    }
    if (not ran_parallel) {
        for (const auto& box: work)
            process_cell(box, evolver);
    }

    LOGGING_PRINTLN("final reach size = " << reach_cells.size());
    LOGGING_PRINTLN("final evolve size = " << evolve_cells.size());
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    lower_evolve(const OvertSetInterfaceType& initial_set,
                 const TimeType& time) const
    -> StorageType {
    LOGGING_SCOPE_CREATE;
    Nat grid_fineness = this->_configuration->maximum_grid_fineness();
    Nat grid_extent = this->_configuration->maximum_grid_extent();
    GridType grid = this->_configuration->grid();
    StorageType initial_cells = this->_storage_factory(grid, this->system());
    StorageType final_cells = this->_storage_factory(grid, this->system());

    LOGGING_PRINTLN("Computing lower evolve set...");

    // Improve accuracy of initial set for lower computations
    ReasoningDetail::storage_adjoin_lower_approximation(initial_cells, initial_set, grid_extent, grid_fineness + 4);
    LOGGING_PRINTLN_AT(1, "initial_cells.size()=" << initial_cells.size());
    ReasoningDetail::for_each_storage_cell(initial_cells, [&](const auto& box) {
        EnclosureType initial_enclosure = _evolver->enclosure(box);
        ListSet<EnclosureType> final_enclosures = _evolver->orbit(initial_enclosure, time, Semantics::LOWER).final();
        for (auto enclosure: final_enclosures) {
            adjoin_outer_approximation_to(enclosure, final_cells, grid_fineness, this->_configuration->paver());
        }
    });
    return final_cells;
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    lower_reach(const OvertSetInterfaceType& initial_set,
                const TimeType& time) const
    -> StorageType {
    LOGGING_SCOPE_CREATE;
    Nat grid_fineness = this->_configuration->maximum_grid_fineness();
    Nat grid_extent = this->_configuration->maximum_grid_extent();
    const GridType& grid = this->_configuration->grid();
    StorageType initial_cells = this->_storage_factory(grid, this->system());
    StorageType reach_cells = this->_storage_factory(grid, this->system());

    LOGGING_PRINTLN("Computing lower reach set...");

    LOGGING_PRINTLN_AT(1, "Adjoining initial set to the grid...");
    // Improve accuracy of initial set for lower computations
    ReasoningDetail::storage_adjoin_lower_approximation(initial_cells, initial_set, grid_extent, grid_fineness + 4);
    LOGGING_PRINTLN_AT(1, "initial_cells.size()=" << initial_cells.size());
    ReasoningDetail::for_each_storage_cell(initial_cells, [&](const auto& box) {
        EnclosureType initial_enclosure = _evolver->enclosure(box);
        ListSet<EnclosureType> reach_enclosures = _evolver->orbit(initial_enclosure, time, Semantics::LOWER).reach();
        for (auto enclosure: reach_enclosures)
            adjoin_outer_approximation_to(enclosure, reach_cells, grid_fineness, this->_configuration->paver());
    });
    return reach_cells;
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    lower_reach_evolve(const OvertSetInterfaceType& initial_set,
                       const TimeType& time) const
    -> Pair<StorageType, StorageType> {
    LOGGING_SCOPE_CREATE;
    Nat grid_fineness = this->_configuration->maximum_grid_fineness();
    Nat grid_extent = this->_configuration->maximum_grid_extent();

    LOGGING_PRINTLN("Computing lower reach evolve...");

    const GridType& grid = this->_configuration->grid();

    StorageType initial_cells = this->_storage_factory(grid, this->system());

    StorageType reach = this->_storage_factory(grid, this->system());
    StorageType evolve_cells = this->_storage_factory(grid, this->system());

    // Improve accuracy of initial set for lower computations
    ReasoningDetail::storage_adjoin_lower_approximation(initial_cells, initial_set, grid_extent, grid_fineness + 4);
    LOGGING_PRINTLN_AT(1, "initial_cells.size()=" << initial_cells.size());

    ReasoningDetail::for_each_storage_cell(initial_cells, [&](const auto& box) {
        EnclosureType initial_enclosure = _evolver->enclosure(box);
        auto orbit = _evolver->orbit(initial_enclosure, time, Semantics::LOWER);
        for (auto enclosure: orbit.reach())
            adjoin_outer_approximation_to(enclosure, reach, grid_fineness, this->_configuration->paver());
        for (auto enclosure: orbit.final())
            adjoin_outer_approximation_to(enclosure, evolve_cells, grid_fineness, this->_configuration->paver());
    });
    return make_pair(reach, evolve_cells);
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    lower_reach(const OvertSetInterfaceType& initial_set) const
    -> StorageType {
    LOGGING_SCOPE_CREATE;
    TimeType transient_time = this->_configuration->transient_time();
    TimeType lock_to_grid_time = this->_configuration->lock_to_grid_time();
    Nat maximum_grid_fineness = this->_configuration->maximum_grid_fineness();
    Nat maximum_grid_extent = this->_configuration->maximum_grid_extent();
    LOGGING_PRINTLN_AT(1, "transient_time=(" << transient_time << ")");
    LOGGING_PRINTLN_AT(1, "lock_to_grid_time=(" << lock_to_grid_time << ")");
    LOGGING_PRINTLN_AT(1, "initial_set=" << initial_set);

    const GridType& grid = this->_configuration->grid();

    Bool has_bounding_domain = this->_configuration->bounding_domain_ptr() != nullptr;

    StorageType bounding = this->_storage_factory(grid, this->system());
    if (has_bounding_domain)
        ReasoningDetail::storage_adjoin_outer_approximation(bounding, this->_configuration->bounding_domain(), maximum_grid_fineness);

    LOGGING_PRINTLN_AT(1, "maximum_grid_extent=" << maximum_grid_extent);
    LOGGING_PRINTLN_AT(1, "bounding_size=" << bounding.size());

    StorageType initial_cells = this->_storage_factory(grid, this->system());
    StorageType evolve_cells = this->_storage_factory(grid, this->system());
    ReasoningDetail::storage_adjoin_lower_approximation(initial_cells, initial_set, maximum_grid_extent, maximum_grid_fineness + 4);

    if (has_bounding_domain)
        ReasoningDetail::storage_restrict(initial_cells, bounding);
    LOGGING_PRINTLN_AT(1, "initial_size=" << initial_cells.size());

    ListSet<EnclosureType> starting_enclosures;

    StorageType reach_cells = this->_storage_factory(grid, this->system());

    if (possibly(transient_time > TimeType(0))) {
        LOGGING_PRINTLN("Computing transient evolution...");

        ListSet<EnclosureType> initial_enclosures;
        ReasoningDetail::for_each_storage_cell(initial_cells,
                                               [&](const auto& box) {
                                                   initial_enclosures.adjoin(_evolver->enclosure(box));
                                               });

        make_lpair(reach_cells, evolve_cells) =
            _reach_evolve_resume(initial_enclosures, transient_time,
                                 maximum_grid_fineness, starting_enclosures, Semantics::LOWER, *_evolver);

        ReasoningDetail::storage_restrict_to_extent(evolve_cells, maximum_grid_extent);
        LOGGING_PRINTLN("Completed restriction to extent.");
        if (has_bounding_domain)
            ReasoningDetail::storage_restrict(reach_cells, bounding);

        LOGGING_PRINTLN_AT(1, "reach size=" << reach_cells.size());
        LOGGING_PRINTLN_AT(1, "evolve size=" << evolve_cells.size());
        LOGGING_PRINTLN("Found " << reach_cells.size() << " cells.");
    }
    LOGGING_PRINTLN("Computing recurrent evolution...");

    while (not ReasoningDetail::storage_is_empty(evolve_cells)) {
        StorageType new_reach_cells = this->_storage_factory(grid, this->system());
        ListSet<EnclosureType> new_evolve_enclosures;
        make_lpair(new_reach_cells, evolve_cells) = _reach_evolve_resume(starting_enclosures, lock_to_grid_time,
                                                                         maximum_grid_fineness, new_evolve_enclosures, Semantics::LOWER, *_evolver);

        LOGGING_PRINTLN_AT(1, "Removing already reached cells...");
        ReasoningDetail::storage_remove(evolve_cells, reach_cells);
        LOGGING_PRINTLN_AT(1, "Restricting to extent...");
        ReasoningDetail::storage_restrict_to_extent(evolve_cells, maximum_grid_extent);
        LOGGING_PRINTLN_AT(1, "Restricting to bounding...");
        if (has_bounding_domain)
            ReasoningDetail::storage_restrict(evolve_cells, bounding);
        LOGGING_PRINTLN_AT(1, "Adjoining...");
        ReasoningDetail::storage_adjoin(reach_cells, new_reach_cells);
        LOGGING_PRINTLN_AT(1, "Found " << new_reach_cells.size() << " cells, of which " << evolve_cells.size() << " are new.");

        starting_enclosures = new_evolve_enclosures;
    }
    ReasoningDetail::storage_recombine(reach_cells);
    ReasoningDetail::storage_restrict_to_extent(reach_cells, maximum_grid_extent);
    if (has_bounding_domain)
        ReasoningDetail::storage_restrict(reach_cells, bounding);

    return reach_cells;
}

inline Natural compute_time_steps(Real time, Real lock_to_grid_time) {
    return ReasoningDetail::whole_time_steps(time, lock_to_grid_time);
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    upper_evolve(const CompactSetInterfaceType& initial_set,
                 const TimeType& time) const
    -> StorageType {
    LOGGING_SCOPE_CREATE;
    const GridType& grid = this->_configuration->grid();
    StorageType evolve_cells = this->_storage_factory(grid, this->system());
    Nat grid_fineness = this->_configuration->maximum_grid_fineness();
    ReasoningDetail::storage_adjoin_outer_approximation(evolve_cells, initial_set, grid_fineness);
    LOGGING_PRINTLN_AT(1, "initial_evolve.size()=" << evolve_cells.size());
    TimeType lock_to_grid_time = this->_configuration->lock_to_grid_time();
    Natural time_steps = compute_time_steps(time, lock_to_grid_time);
    TimeType remainder_time = time - (time_steps * lock_to_grid_time);
    LOGGING_PRINTLN_AT(1, "real_time=" << time);
    LOGGING_PRINTLN_AT(1, "time_steps=" << time_steps << "  lock_to_grid_time=" << lock_to_grid_time);

    for (Natural i = 0u; i != time_steps; ++i) {
        LOGGING_PRINTLN_AT(2, "computing " << i + 1u << "-th reachability step...");
        evolve_cells = this->_upper_evolve(evolve_cells, lock_to_grid_time, grid_fineness, *_evolver);
    }
    LOGGING_PRINTLN_AT(1, "remainder_time=" << remainder_time);
    if (not ReasoningDetail::storage_is_empty(evolve_cells) and possibly(remainder_time > 0)) {
        LOGGING_PRINTLN_AT(1, "computing evolution for remainder time...");
        evolve_cells = this->_upper_evolve(evolve_cells, remainder_time, grid_fineness, *_evolver);
    }
    ReasoningDetail::storage_recombine(evolve_cells);
    LOGGING_PRINTLN("final_evolve.size()=" << evolve_cells.size());
    return evolve_cells;
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    upper_reach(const CompactSetInterfaceType& initial_set,
                const TimeType& time) const
    -> StorageType {
    LOGGING_SCOPE_CREATE;
    LOGGING_PRINTLN_AT(1, "initial_set=" << initial_set);
    const GridType& grid = this->_configuration->grid();
    StorageType evolve_cells = this->_storage_factory(grid, this->system());
    Nat grid_fineness = this->_configuration->maximum_grid_fineness();
    LOGGING_PRINTLN_AT(2, "grid_fineness=" << grid_fineness);
    ReasoningDetail::storage_adjoin_outer_approximation(evolve_cells, initial_set, grid_fineness);
    LOGGING_PRINTLN("initial size = " << evolve_cells.size());
    StorageType reach_cells(evolve_cells);
    LOGGING_PRINTLN("reach size =" << reach_cells.size());
    TimeType lock_to_grid_time = this->_configuration->lock_to_grid_time();
    Natural time_steps = compute_time_steps(time, lock_to_grid_time);
    TimeType remainder_time = time - time_steps * lock_to_grid_time;

    LOGGING_PRINTLN_AT(1, "time=" << time);
    LOGGING_PRINTLN_AT(1, "time_steps=" << time_steps << "  lock_to_grid_time=" << lock_to_grid_time);
    StorageType found_cells = this->_storage_factory(grid, this->system());
    StorageType accumulated_evolve_cells = this->_storage_factory(grid, this->system());
    LOGGING_PRINTLN("Computing time steps...")

    for (Natural i = 0u; i != time_steps; ++i) {
        ReasoningDetail::storage_adjoin(accumulated_evolve_cells, found_cells);
        LOGGING_PRINTLN_AT(1, "computing " << i + 1u << "-th reachability step...");
        this->_adjoin_upper_reach_evolve(found_cells, evolve_cells, evolve_cells, lock_to_grid_time, grid_fineness, *_evolver);
        if constexpr (not std::is_same_v<StorageType, HybridAugmentedPavingValue>)
            LOGGING_PRINTLN_AT(1, "evolve bounding box = " << evolve_cells.state_paving().bounding_box());
        LOGGING_PRINTLN_AT(1, "found.size()=" << found_cells.size());
        LOGGING_PRINTLN_AT(1, "evolve.size()=" << evolve_cells.size());
        ReasoningDetail::storage_remove(evolve_cells, accumulated_evolve_cells);
        ReasoningDetail::storage_adjoin(reach_cells, found_cells);
        ReasoningDetail::storage_adjoin(accumulated_evolve_cells, evolve_cells);
        LOGGING_PRINTLN_AT(1, "found " << found_cells.size() << " cells, with " << evolve_cells.size() << " new intermediate.");
        if (ReasoningDetail::storage_is_empty(evolve_cells))
            break;
    }
    LOGGING_PRINTLN("remainder_time=" << remainder_time);
    if (not ReasoningDetail::storage_is_empty(evolve_cells) and possibly(remainder_time > 0)) {
        LOGGING_PRINTLN("computing evolution for remainder time...");
        this->_adjoin_upper_reach_evolve(found_cells, accumulated_evolve_cells, evolve_cells, remainder_time, grid_fineness, *_evolver);
        ReasoningDetail::storage_adjoin(reach_cells, found_cells);
    }
    // This last step is necessary to add the final set to the result.
    ReasoningDetail::storage_adjoin(reach_cells, evolve_cells);
    ReasoningDetail::storage_recombine(reach_cells);
    LOGGING_PRINTLN("final_reach size = " << reach_cells.size());
    return reach_cells;
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    upper_reach_evolve(const CompactSetInterfaceType& initial_set,
                       const TimeType& time) const
    -> Pair<StorageType, StorageType> {
    LOGGING_SCOPE_CREATE;
    LOGGING_PRINTLN_AT(1, "initial_set=" << initial_set);
    const GridType& grid = this->_configuration->grid();
    StorageType evolve_cells = this->_storage_factory(grid, this->system());
    Nat grid_fineness = this->_configuration->maximum_grid_fineness();
    LOGGING_PRINTLN_AT(1, "grid_fineness=" << grid_fineness);
    ReasoningDetail::storage_adjoin_outer_approximation(evolve_cells, initial_set, grid_fineness);

    LOGGING_PRINTLN_AT(1, "initial_evolve" << evolve_cells);
    StorageType reach_cells(evolve_cells);
    LOGGING_PRINTLN_AT(1, "reach=" << reach_cells);
    TimeType lock_to_grid_time = this->_configuration->lock_to_grid_time();
    Natural time_steps = compute_time_steps(time, lock_to_grid_time);
    TimeType remainder_time = time - time_steps * lock_to_grid_time;
    LOGGING_PRINTLN("time=" << time);
    LOGGING_PRINTLN("time_steps=" << time_steps << "  lock_to_grid_time=" << lock_to_grid_time);

    LOGGING_PRINTLN("Computing reachability steps...");

    StorageType found_cells = this->_storage_factory(grid, this->system());
    for (Natural i = 0u; i != time_steps; ++i) {
        LOGGING_PRINTLN_AT(1, "computing " << i + 1u << "-th reachability step...");
        this->_adjoin_upper_reach_evolve(found_cells, evolve_cells, evolve_cells, lock_to_grid_time, grid_fineness, *_evolver);
        LOGGING_PRINTLN_AT(2, "evolve.size()=" << evolve_cells.size());
        ReasoningDetail::storage_adjoin(reach_cells, found_cells);
        LOGGING_PRINTLN_AT(1, "found " << found_cells.size() << " cells.");
    }
    LOGGING_PRINTLN("remainder_time=" << remainder_time);
    if (not ReasoningDetail::storage_is_empty(evolve_cells) and possibly(remainder_time > 0)) {
        LOGGING_PRINTLN("computing evolution for remainder time...");
        this->_adjoin_upper_reach_evolve(found_cells, evolve_cells, evolve_cells, remainder_time, grid_fineness, *_evolver);
        ReasoningDetail::storage_adjoin(reach_cells, found_cells);
    }
    ReasoningDetail::storage_recombine(reach_cells);
    LOGGING_PRINTLN("reach=" << reach_cells);
    ReasoningDetail::storage_recombine(evolve_cells);
    LOGGING_PRINTLN("evolve=" << evolve_cells);
    return std::make_pair(reach_cells, evolve_cells);
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    outer_chain_reach(const CompactSetInterfaceType& initial_set) const
    -> StorageType {
    LOGGING_SCOPE_CREATE;

    auto overspill_policy = this->_configuration->outer_overspill_policy();

    TimeType transient_time = this->_configuration->transient_time();
    TimeType lock_to_grid_time = this->_configuration->lock_to_grid_time();
    Nat maximum_grid_fineness = this->_configuration->maximum_grid_fineness();
    Nat maximum_grid_extent = this->_configuration->maximum_grid_extent();
    LOGGING_PRINTLN_AT(1, "transient_time=(" << transient_time << ")");
    LOGGING_PRINTLN_AT(1, "lock_to_grid_time=(" << lock_to_grid_time << ")");
    LOGGING_PRINTLN_AT(1, "initial_set=" << initial_set);

    const GridType& grid = this->_configuration->grid();

    Bool has_bounding_domain = this->_configuration->bounding_domain_ptr() != nullptr;
    StorageType bounding = this->_storage_factory(grid, this->system());
    if (has_bounding_domain)
        ReasoningDetail::storage_adjoin_outer_approximation(bounding, this->_configuration->bounding_domain(), maximum_grid_fineness);

    LOGGING_PRINTLN_AT(1, "maximum_grid_extent=" << maximum_grid_extent);
    LOGGING_PRINTLN_AT(1, "bounding_size=" << bounding.size());

    StorageType initial_cells = this->_storage_factory(grid, this->system());
    ReasoningDetail::storage_adjoin_outer_approximation(initial_cells, initial_set, maximum_grid_fineness);
    _checked_restriction(initial_cells, bounding, overspill_policy);
    LOGGING_PRINTLN_AT(1, "initial_size=" << initial_cells.size());

    StorageType reach_cells = this->_storage_factory(grid, this->system());
    StorageType evolve_cells = this->_storage_factory(grid, this->system());
    if (definitely(transient_time > TimeType(0))) {
        LOGGING_PRINTLN("Computing transient evolution...");
        this->_adjoin_upper_reach_evolve(reach_cells, evolve_cells, initial_cells, transient_time, maximum_grid_fineness, *_evolver);
        _checked_restriction(evolve_cells, bounding, overspill_policy);
        ReasoningDetail::storage_recombine(evolve_cells);
        ReasoningDetail::storage_mince(evolve_cells, maximum_grid_fineness);
        LOGGING_PRINTLN_AT(1, "transient_reach_size=" << reach_cells.size());
        LOGGING_PRINTLN_AT(1, "transient evolve_size=" << evolve_cells.size());
        LOGGING_PRINTLN_AT(1, "found " << reach_cells.size() << " cells.");
    } else {
        evolve_cells = initial_cells;
    }

    LOGGING_PRINTLN("Computing recurrent evolution...");
    StorageType starting_cells = evolve_cells;
    StorageType accumulated_evolve_cells = evolve_cells;

    Nat stage = 0;
    while (not ReasoningDetail::storage_is_empty(starting_cells)) {
        ++stage;
        LOGGING_PRINTLN_AT(1, "stage=" << std::setw(3) << stage << " #starting=" << std::setw(4) << std::left << starting_cells.size() << " #reached=" << std::setw(4) << std::left << reach_cells.size() << " #evolved=" << std::setw(4) << std::left << accumulated_evolve_cells.size());

        this->_adjoin_upper_reach_evolve(reach_cells, evolve_cells, starting_cells,
                                         lock_to_grid_time, maximum_grid_fineness, *_evolver);
        LOGGING_PRINTLN_AT(2, "reach.size()=" << reach_cells.size());
        LOGGING_PRINTLN_AT(2, "evolve.size()=" << evolve_cells.size());
        _checked_restriction(evolve_cells, bounding, overspill_policy);
        starting_cells = evolve_cells;
        ReasoningDetail::storage_remove(starting_cells, accumulated_evolve_cells);
        ReasoningDetail::storage_adjoin(accumulated_evolve_cells, starting_cells);
        ReasoningDetail::storage_mince(starting_cells, maximum_grid_fineness);
        LOGGING_PRINTLN_AT(2, "evolved to " << evolve_cells.size() << " cells, of which " << starting_cells.size() << " are new.");
    }
    _checked_restriction(reach_cells, bounding, overspill_policy);
    return reach_cells;
}

template<class SYS>
auto ReachabilityAnalyser<SYS>::
    verify_safety(const CompactSetInterfaceType& initial_set,
                  const OpenSetInterfaceType& safe_set) const
    -> SafetyCertificateType {
    LOGGING_SCOPE_CREATE;
    const RegularLocatedSetInterfaceType* bounded_safe_set_ptr = dynamic_cast<const RegularLocatedSetInterfaceType*>(&safe_set);
    const GridType& grid = this->_configuration->grid();
    auto make_safe_cells = [&]() -> StorageType {
        if constexpr (std::is_same_v<StorageType, HybridAugmentedPavingValue>) {
            const Hybrid<Paving> safe_state = inner_approximation(
                *bounded_safe_set_ptr, grid,
                [](const Grid& g) { return Paving(new BinaryTreePaving(g)); },
                this->_configuration->maximum_grid_fineness());
            return make_analysis_paving<StorageType>(safe_state, this->system());
        } else {
            auto safe_paving = inner_approximation(*bounded_safe_set_ptr, grid, this->_configuration->maximum_grid_fineness());
            return make_analysis_paving<StorageType>(safe_paving, this->system());
        }
    };
    StorageType safe_cells = make_safe_cells();

    auto overspill_policy = ChainOverspillPolicy::ERROR;

    TimeType transient_time = this->_configuration->transient_time();
    TimeType lock_to_grid_time = this->_configuration->lock_to_grid_time();
    Nat maximum_grid_fineness = this->_configuration->maximum_grid_fineness();
    Nat maximum_grid_extent = this->_configuration->maximum_grid_extent();
    LOGGING_PRINTLN_AT(1, "transient_time=(" << transient_time << ")");
    LOGGING_PRINTLN_AT(1, "lock_to_grid_time=(" << lock_to_grid_time << ")");
    LOGGING_PRINTLN_AT(1, "initial_set=" << initial_set);

    Bool has_bounding_domain = this->_configuration->bounding_domain_ptr() != nullptr;
    StorageType bounding = this->_storage_factory(grid, this->system());
    if (has_bounding_domain)
        ReasoningDetail::storage_adjoin_outer_approximation(bounding, this->_configuration->bounding_domain(), maximum_grid_fineness);

    LOGGING_PRINTLN_AT(1, "maximum_grid_extent=" << maximum_grid_extent);
    LOGGING_PRINTLN_AT(1, "bounding_size=" << bounding.size());

    StorageType initial_cells = this->_storage_factory(grid, this->system());
    ReasoningDetail::storage_adjoin_outer_approximation(initial_cells, initial_set, maximum_grid_fineness);
    _checked_restriction(initial_cells, bounding, overspill_policy);
    LOGGING_PRINTLN_AT(1, "initial_size=" << initial_cells.size());

    if (not subset(initial_cells, safe_cells)) {
        return SafetyCertificateType{indeterminate, initial_cells, safe_cells};
    }

    StorageType reach_cells = this->_storage_factory(grid, this->system());
    StorageType evolve_cells = this->_storage_factory(grid, this->system());
    if (definitely(transient_time > 0)) {
        LOGGING_PRINTLN("Computing transient evolution...");
        this->_adjoin_upper_reach_evolve(reach_cells, evolve_cells, initial_cells, transient_time, maximum_grid_fineness, *_evolver);
        _checked_restriction(initial_cells, bounding, overspill_policy);
        ReasoningDetail::storage_recombine(evolve_cells);
        ReasoningDetail::storage_mince(evolve_cells, maximum_grid_fineness);
        LOGGING_PRINTLN_AT(1, "transient_reach_size=" << reach_cells.size());
        LOGGING_PRINTLN_AT(1, "transient evolve_size=" << evolve_cells.size());
        LOGGING_PRINTLN_AT(1, "found " << reach_cells.size() << " cells.");
    } else {
        evolve_cells = initial_cells;
    }

    LOGGING_PRINTLN("Computing recurrent evolution...");
    StorageType starting_cells = evolve_cells;
    StorageType accumulated_evolve_cells = evolve_cells;

    Nat stage = 0;
    while (not ReasoningDetail::storage_is_empty(starting_cells)) {
        ++stage;
        LOGGING_PRINTLN_AT(1, "stage=" << std::setw(3) << stage << " #starting=" << std::setw(4) << std::left << starting_cells.size() << " #reached=" << std::setw(4) << std::left << reach_cells.size() << " #evolved=" << std::setw(4) << std::left << accumulated_evolve_cells.size());

        if (not subset(reach_cells, safe_cells)) {
            return SafetyCertificateType{indeterminate, reach_cells, safe_cells};
        }

        this->_adjoin_upper_reach_evolve(reach_cells, evolve_cells, starting_cells,
                                         lock_to_grid_time, maximum_grid_fineness, *_evolver);
        LOGGING_PRINTLN_AT(2, "reach.size()=" << reach_cells.size());
        LOGGING_PRINTLN_AT(2, "evolve.size()=" << evolve_cells.size());
        _checked_restriction(initial_cells, bounding, overspill_policy);
        starting_cells = evolve_cells;
        ReasoningDetail::storage_remove(starting_cells, accumulated_evolve_cells);
        ReasoningDetail::storage_adjoin(accumulated_evolve_cells, starting_cells);
        ReasoningDetail::storage_mince(starting_cells, maximum_grid_fineness);
        LOGGING_PRINTLN_AT(2, "evolved to " << evolve_cells.size() << " cells, of which " << starting_cells.size() << " are new.");
    }
    _checked_restriction(reach_cells, bounding, overspill_policy);
    return SafetyCertificateType{true, reach_cells, safe_cells};
}

template<class SYS>
Void ReachabilityAnalyser<SYS>::
    _checked_restriction(StorageType& set, const StorageType& bounding, ChainOverspillPolicy policy) const {
    const GridType& grid = this->_configuration->grid();
    StorageType set_copy = this->_storage_factory(grid, this->system());

    if (policy != ChainOverspillPolicy::IGNORE) {
        set_copy = set;
    }
    ReasoningDetail::storage_restrict_to_extent(set, this->_configuration->maximum_grid_extent());
    if (not ReasoningDetail::storage_is_empty(bounding))
        ReasoningDetail::storage_restrict(set, bounding);

    if (policy != ChainOverspillPolicy::IGNORE) {
        ReasoningDetail::storage_remove(set_copy, set);
        if (not ReasoningDetail::storage_is_empty(set_copy)) {
            if (policy == ChainOverspillPolicy::WARNING) {
                ARIADNE_WARN("The computed chain reach has been restricted, an outer approximation is not returned.");
            }
            if (policy == ChainOverspillPolicy::ERROR)
                ARIADNE_THROW(OuterChainOverspill, "outer_chain_reach", "The computed chain reach has been restricted.");
        }
    }
}

template<class SYS>
Void ReachabilityAnalyserConfiguration<SYS>::set_grid(const GridType& grid) {
    this->_grid_ptr.reset(new GridType(grid));
}

template<class SYS>
Void ReachabilityAnalyserConfiguration<SYS>::set_bounding_domain(const BoundingDomainType& bounding_domain) {
    this->_bounding_domain_ptr.reset(new BoundingDomainType(bounding_domain));
}

template<class SYS>
ReachabilityAnalyserConfiguration<SYS>::ReachabilityAnalyserConfiguration(ReachabilityAnalyser<SYS>& analyser)
    : _analyser(analyser), _paver(new SubdivisionPaver()) {
    set_transient_time(0);
    set_lock_to_grid_time(1);
    set_maximum_grid_fineness(3);
    set_maximum_grid_extent(16);
    set_grid(GridType(_analyser.system()));
    set_outer_overspill_policy(ChainOverspillPolicy::ERROR);
}

template<class SYS>
OutputStream&
    ReachabilityAnalyserConfiguration<SYS>::_write(OutputStream& os) const {
    os << "ReachabilityAnalyserSettings"
       << "(\n  transient_time=" << transient_time()
       << ",\n  lock_to_grid_time=" << lock_to_grid_time()
       << ",\n  maximum_grid_fineness=" << maximum_grid_fineness()
       << ",\n  maximum_grid_extent=" << maximum_grid_extent()
       << ",\n  bounding_domain=";
    if (bounding_domain_ptr())
        os << bounding_domain();
    else
        os << "none";
    os << ",\n  paver=" << paver()
       << ",\n  grid=" << grid()
       << "\n)\n";
    return os;
}

} // namespace Ariadne

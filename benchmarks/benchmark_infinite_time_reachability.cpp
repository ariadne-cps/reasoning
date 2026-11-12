/***************************************************************************
 *            benchmark_infinite_time_reachability.cpp
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

/** @file benchmarks/benchmark_infinite_time_reachability.cpp
 *  @brief Benchmark infinite-time lower and outer-chain reachability computations.
 */

// Whole-analysis infinite-time lower and outer-chain reachability benchmark.
// Workload matches tests/test_infinite_time_reachability.cpp.
// The analyser selects the backend for every intermediate Paving.
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "algebra/algebra.hpp"
#include "evolution/enclosure.hpp"
#include "evolution/orbit.hpp"
#include "evolution/vector_field_evolver.hpp"
#include "function/formula.hpp"
#include "function/taylor_function.hpp"
#include "integration/integrator.hpp"
#include "logging/progress_indicator.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/robdd_paving.hpp"
#include "reasoning/reachability_analyser.hpp"
#include "set/function_set.hpp"
#include "threading/thread_manager.hpp"
#include "threading/threading_command_line_interface.hpp"

#include "ariadne-benchmarking.hpp"

using namespace Ariadne;
namespace Bench = Ariadne::Benchmarking;

namespace {
using Evolver = VectorFieldEvolver;
using System = Evolver::SystemType;
using Analyser = ReachabilityAnalyser<System>;
using Storage = Analyser::StorageType;
using BoundingDomain = Analyser::BoundingDomainType;

System make_system() {
    RealVariable x("x"), y("y");
    Real a(-0.5_x), b(1.0_x);
    return System({dot(x) = -a * x - b * y, dot(y) = b * x + 2 * a * y});
}
} // namespace

int main(int argc, const char* argv[]) {
    try {
        Bench::Parameter<unsigned> fineness("maximum-grid-fineness",
                                            "Maximum grid-relative fineness", {3u});
        Bench::Parameter<unsigned> samples("samples",
                                           "Whole-analysis measured samples", {3u});
        Bench::Parameter<std::string> backend("backend",
                                              "Actual storage backend (tree or robdd)", {"tree", "robdd"});
        Bench::Parameter<std::string> operation("operation",
                                                "Reachability operation (lower_reach or outer_chain_reach)",
                                                {"lower_reach", "outer_chain_reach"});
        Bench::Matrix matrix;
        // Backend varies fastest for each operation, so the tree reference
        // is available before the ROBDD geometry check.
        matrix.add(fineness).add(operation).add(backend).add(samples);
        Bench::CommandLine cli;
        ThreadManager::instance();
        cli.add(threading_command_line_contribution());
        cli.add_parameters(matrix);
        if (!cli.acquire(argc, argv))
            return 0;

        Bench::Report report("Infinite-time reachability", cli.summary_fields());
        report.set_time_unit(Bench::ReportTimeUnit::Seconds);
        report.set_dimension_columns({{"Backend"}, {"Maximum grid fineness"}, {"Concurrency"}, {"Operation"}});
        report.set_parameter_header(matrix.header());
        Bench::Runner runner(cli, matrix.combinations());
        // One global progress indicator counting completed executions over
        // the entire matrix, including warm-ups and individual samples.
        std::size_t total_executions = 0u;
        matrix.for_each([&](const Bench::Configuration& config) {
            const std::size_t n = config.value(samples);
            const std::size_t rounds = n + static_cast<std::size_t>(cli.warmups());
            if (n == 0u or rounds < n or
                total_executions > std::numeric_limits<std::size_t>::max() - rounds)
                throw std::overflow_error("Invalid benchmark execution count");
            total_executions += rounds;
        });
        std::size_t completed_executions = 0u;
        ProgressIndicator overall_progress(static_cast<double>(total_executions));
        // Keep the overall indicator at level 1. Each configuration gets
        // one nested held progress indicator at level 2, released before
        // entering the next configuration.
        LogScopeManager total_scope("reachability-benchmark-total", 0u);
        if (not Logger::instance().is_muted_at(0))
            Logger::instance().hold(total_scope.scope(),
                                    "[ " + std::string(1u, overall_progress.symbol()) +
                                        " ] 0% (0/" + std::to_string(total_executions) +
                                        " executions completed)");

        System system = make_system();
        RealVariable x("x"), y("y");
        RealExpressionBoundedConstraintSet symbolic_initial(
            {1.98_dec <= x <= 1.99_dec, 0.01_dec <= y <= 0.02_dec});
        BoundedConstraintSet initial(
            symbolic_initial.euclidean_set(system.state_space()));
        BoundingDomain bounding({
            {-4, 4},
            {-4, 4}
        });
        // Tree and ROBDD must compute precisely the same dyadic region.
        // The reference is materialized only after measurement.
        std::optional<BinaryTreePaving> tree_reference;
        std::optional<unsigned> reference_fineness;
        std::optional<std::string> reference_operation;

        matrix.for_each([&](const Bench::Configuration& config) {
            // We stay at level 1 so the global indicator is always updated
            // at level 1; explicit messages for this step use level 2.
            const std::string step_label =
                config.value(operation) + " backend=" + config.value(backend) +
                " fineness=" + std::to_string(config.value(fineness));
            if (not Logger::instance().is_muted_at(0))
                Logger::instance().hold(total_scope.scope(),
                                        "[ " + std::string(1u, overall_progress.symbol()) +
                                            " ] " + std::to_string(overall_progress.percentage()) +
                                            "% (" + std::to_string(completed_executions) + "/" +
                                            std::to_string(total_executions) + "); step " +
                                            std::to_string(config.ordinal()) + "/" +
                                            std::to_string(config.count()) + ": " + step_label);
            runner.configuration_started(config);
            unsigned f = config.value(fineness), n = config.value(samples);
            const std::string implementation = config.value(backend);
            const std::string selected_operation = config.value(operation);
            if (selected_operation != "lower_reach" and
                selected_operation != "outer_chain_reach")
                throw std::invalid_argument(
                    "Unsupported operation: lower_reach or outer_chain_reach");
            if (not reference_fineness or *reference_fineness != f or
                not reference_operation or *reference_operation != selected_operation) {
                tree_reference.reset();
                reference_fineness = f;
                reference_operation = selected_operation;
            }
            if (implementation != "tree" and implementation != "robdd")
                throw std::invalid_argument(
                    "Unsupported backend: tree or robdd. CachedTree lacks mutable PavingInterface");
            if (n == 0u || f > 16u)
                throw std::invalid_argument("samples must be >0; fineness <=16");
            std::unique_ptr<Analyser> analyser;
            std::unique_ptr<Storage> result;
            std::shared_ptr<RobddManager> manager;
            std::size_t completed_rounds = 0u;
            const std::size_t warmups = cli.warmups();
            const std::size_t total_rounds = warmups + static_cast<std::size_t>(n);
            ProgressIndicator round_progress(static_cast<double>(total_rounds));
            LogScopeManager round_scope(
                "reachability-benchmark-rounds-" + std::to_string(config.ordinal()), 0u);
            const std::string context = "[" + std::to_string(config.ordinal()) +
                                        "/" + std::to_string(config.count()) + "] " +
                                        selected_operation + " backend=" + implementation +
                                        " fineness=" + std::to_string(f) +
                                        " concurrency=" +
                                        std::to_string(ThreadManager::instance().concurrency());
            // Hold a second progress line *at* level 2, but do not
            // increase the ambient logging level for the whole callback:
            // the outer indicator must continue to update at level 1.
            auto update_round_progress = [&] {
                if (not Logger::instance().is_muted_at(1u)) {
                    LOGGING_RUN_AT(1,
                                   Logger::instance().hold(round_scope.scope(),
                                                           "[ " + std::string(1u, round_progress.symbol()) +
                                                               " ] " + std::to_string(round_progress.percentage()) +
                                                               "% (" + std::to_string(completed_rounds) + "/" +
                                                               std::to_string(total_rounds) + ") " + context))
                }
            };
            LOGGING_PRINTLN_AT(1, context << " starting, warmups=" << warmups
                                          << ", samples=" << n)
            update_round_progress();
            auto timings = runner.run(selected_operation, n, [&] {
                    GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-2_pr));
                    Evolver evolver(system,integrator);
                    analyser=std::make_unique<Analyser>(evolver);
                    manager.reset();
                    if(implementation=="robdd") {
                        manager=std::make_shared<RobddManager>();
                        analyser->set_paving_factory([owner=manager]( const Grid & grid_value) {
                            return Paving(new RobddPaving(owner,grid_value));
                        });
                    } else {
                        analyser->set_paving_factory([]( const Grid & grid_value) {
                            return Paving(new BinaryTreePaving(grid_value));
                        });
                    }
                    analyser->configuration().set_maximum_grid_fineness(f);
                    if(selected_operation=="outer_chain_reach") {
                        analyser->configuration().set_transient_time(12.0_dec);
                        analyser->configuration().set_lock_to_grid_time(6.0_dec);
                    } else {
                        analyser->configuration().set_transient_time(4.0_dec);
                    }
                    analyser->configuration().set_bounding_domain_ptr(
                        std::make_shared<BoundingDomain>(bounding));
                    result.reset();
                    // Preparation and progress output are outside timing.
                    const std:: size_t next=completed_rounds+1u;
                    const bool warmup=next<=warmups;
                    LOGGING_PRINTLN_AT(1,context << " round " << next << "/"
                        << total_rounds << (warmup ? " [warmup]" : " [sample]")
                        << " running...") }, [&] {
                    // The algorithm and its own nested logs start at
                    // level 3: level 1 is global progress, level 2 is
                    // this benchmark configuration. RUN_AT does not restore
                    // the level on exception, so restore it explicitly.
                    try {
                        LOGGING_RUN_AT(2,
                            result=std::make_unique<Storage>(
                                selected_operation=="lower_reach"
                                    ? analyser->lower_reach(initial)
                                    : analyser->outer_chain_reach(initial)))
                    } catch(...) {
                        Logger::instance().decrease_level(2u);
                        throw;
                    } }, [&] {
                    if(result==nullptr || result->is_empty())
                        throw std::runtime_error("Empty reachable set");
                    Bench::keep_alive(result->size());
                    ++completed_rounds;
                    ++completed_executions;
                    round_progress.update_current(static_cast<double>(completed_rounds));
                    overall_progress.update_current(
                        static_cast<double>(completed_executions));
                    if(not Logger::instance().is_muted_at(0))
                        Logger::instance().hold(total_scope.scope(),
                            "[ " + std::string(1u,overall_progress.symbol()) +
                            " ] " + std::to_string(overall_progress.percentage()) +
                            "% (" + std::to_string(completed_executions) + "/" +
                            std::to_string(total_executions) + "); step " +
                            std::to_string(config.ordinal()) + "/" +
                            std::to_string(config.count()) + ": " + step_label);
                    update_round_progress();
                    LOGGING_PRINTLN_AT(1,context << " round " << completed_rounds
                        << "/" << total_rounds << " completed") });

            // Compare the final reached geometry, not the backend-specific
            // enabled-leaf counts. This conversion is OUTSIDE timed work.
            BinaryTreePaving materialized(result->state_paving().grid());
            const auto& state = result->state_paving();
            for (auto it = state.begin(), end = state.end(); it != end; ++it)
                materialized.adjoin(*it);
            if (implementation == "tree") {
                tree_reference = materialized;
            } else {
                if (not tree_reference)
                    throw std::runtime_error("Tree reference must precede ROBDD for each operation/fineness");
                if (not subset(materialized, *tree_reference) or not subset(*tree_reference, materialized))
                    throw std::runtime_error("ROBDD reached a different geometric set");
            }
            report.add_row({implementation, std::to_string(f),
                            std::to_string(ThreadManager::instance().concurrency()),
                            selected_operation},
                           Bench::summarize(timings));
            runner.configuration_completed(config);
            LOGGING_PRINTLN_AT(1, context << " completed")
        });
        Logger::instance().release(total_scope.scope());
        report.print();
        return 0;
    } catch (const std::exception& e) {
        LOGGING_PRINTLN("benchmark_infinite_time_reachability: " << e.what())
        return 1;
    }
}

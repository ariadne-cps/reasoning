// Whole-analysis infinite-time lower and outer-chain reachability benchmark.
// Workload matches tests/test_infinite_time_reachability.cpp.
// The analyser selects the backend for every intermediate Paving.
#include "ariadne-benchmarking.hpp"
#include "threading/thread_manager.hpp"
#include "threading/threading_command_line_interface.hpp"
#include "function/taylor_function.hpp"
#include "function/formula.hpp"
#include "algebra/algebra.hpp"
#include "set/function_set.hpp"
#include "evolution/enclosure.hpp"
#include "evolution/orbit.hpp"
#include "evolution/vector_field_evolver.hpp"
#include "reasoning/reachability_analyser.hpp"
#include "integration/integrator.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/robdd_paving.hpp"
#include "logging/progress_indicator.hpp"
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Ariadne;
namespace Bench=Ariadne::Benchmarking;

namespace {
using Evolver=VectorFieldEvolver;
using System=Evolver::SystemType;
using Analyser=ReachabilityAnalyser<System>;
using Storage=Analyser::StorageType;
using BoundingDomain=Analyser::BoundingDomainType;

System make_system() {
    RealVariable x("x"),y("y");
    Real a(-0.5_x),b(1.0_x);
    return System({dot(x)=-a*x-b*y,dot(y)=b*x+2*a*y});
}
} // namespace

int main(int argc,char const* argv[]) {
    try {
        Bench::Parameter<unsigned> fineness("maximum-grid-fineness",
            "Maximum grid-relative fineness",{3u});
        Bench::Parameter<unsigned> samples("samples",
            "Whole-analysis measured samples",{3u});
        Bench::Parameter<std::string> backend("backend",
            "Actual storage backend (tree or robdd)",{"tree","robdd"});
        Bench::Parameter<std::string> operation("operation",
            "Reachability operation (lower_reach or outer_chain_reach)",
            {"lower_reach","outer_chain_reach"});
        Bench::Matrix matrix;
        // Backend varies fastest for each operation, so the tree reference
        // is available before the ROBDD geometry check.
        matrix.add(fineness).add(operation).add(backend).add(samples);
        Bench::CommandLine cli;
        ThreadManager::instance();
        cli.add(threading_command_line_contribution());
        cli.add_parameters(matrix);
        if(!cli.acquire(argc,argv)) return 0;

        Bench::Report report("Infinite-time reachability",cli.summary_fields());
        report.set_time_unit(Bench::ReportTimeUnit::Seconds);
        report.set_dimension_columns({
            {"Backend"},{"Maximum grid fineness"},{"Concurrency"},{"Operation"}});
        report.set_parameter_header(matrix.header());
        Bench::Runner runner(cli,matrix.combinations());
        // One global progress indicator counting completed executions over
        // the entire matrix, including warm-ups and individual samples.
        std::size_t total_executions=0u;
        matrix.for_each([&](Bench::Configuration const& config) {
            std::size_t const n=config.value(samples);
            std::size_t const rounds=n+static_cast<std::size_t>(cli.warmups());
            if(n==0u or rounds<n or
               total_executions>std::numeric_limits<std::size_t>::max()-rounds)
                throw std::overflow_error("Invalid benchmark execution count");
            total_executions+=rounds;
        });
        std::size_t completed_executions=0u;
        ProgressIndicator overall_progress(static_cast<double>(total_executions));
        // A single held line at level 1; level 2 logs every case.
        // No nested progress indicators or scope-dependent global updates.
        LogScopeManager total_scope("reachability-benchmark-total",0u);
        if(not Logger::instance().is_muted_at(0))
            Logger::instance().hold(total_scope.scope(),
                "[ " + std::string(1u,overall_progress.symbol()) +
                " ] 0% (0/" + std::to_string(total_executions) +
                " executions completed)");

        System system=make_system();
        RealVariable x("x"),y("y");
        RealExpressionBoundedConstraintSet symbolic_initial(
            {1.98_dec<=x<=1.99_dec,0.01_dec<=y<=0.02_dec});
        BoundedConstraintSet initial(
            symbolic_initial.euclidean_set(system.state_space()));
        BoundingDomain bounding({{-4,4},{-4,4}});
        // Tree and ROBDD must compute precisely the same dyadic region.
        // The reference is materialized only after measurement.
        std::optional<BinaryTreePaving> tree_reference;
        std::optional<unsigned> reference_fineness;
        std::optional<std::string> reference_operation;

        matrix.for_each([&](Bench::Configuration const& config) {
            // We stay at level 1 so the global indicator is always updated
            // at level 1; explicit messages for this step use level 2.
            std::string const step_label=
                config.value(operation)+" backend="+config.value(backend)+
                " fineness="+std::to_string(config.value(fineness));
            if(not Logger::instance().is_muted_at(0))
                Logger::instance().hold(total_scope.scope(),
                    "[ " + std::string(1u,overall_progress.symbol()) +
                    " ] " + std::to_string(overall_progress.percentage()) +
                    "% (" + std::to_string(completed_executions) + "/" +
                    std::to_string(total_executions) + "); step " +
                    std::to_string(config.ordinal()) + "/" +
                    std::to_string(config.count()) + ": " + step_label);
            runner.configuration_started(config);
            unsigned f=config.value(fineness),n=config.value(samples);
            std::string const implementation=config.value(backend);
            std::string const selected_operation=config.value(operation);
            if(selected_operation!="lower_reach" and
               selected_operation!="outer_chain_reach")
                throw std::invalid_argument(
                    "Unsupported operation: lower_reach or outer_chain_reach");
            if(not reference_fineness or *reference_fineness!=f or
               not reference_operation or *reference_operation!=selected_operation) {
                tree_reference.reset();
                reference_fineness=f;
                reference_operation=selected_operation;
            }
            if(implementation!="tree" and implementation!="robdd")
                throw std::invalid_argument(
                    "Unsupported backend: tree or robdd. CachedTree lacks mutable PavingInterface");
            if(n==0u || f>16u)
                throw std::invalid_argument("samples must be >0; fineness <=16");
            std::unique_ptr<Analyser> analyser;
            std::unique_ptr<Storage> result;
            std::shared_ptr<RobddManager> manager;
            std::size_t completed_rounds=0u;
            std::size_t const warmups=cli.warmups();
            std::size_t const total_rounds=warmups+static_cast<std::size_t>(n);
            std::string const context="[" + std::to_string(config.ordinal()) +
                "/" + std::to_string(config.count()) + "] " +
                selected_operation + " backend=" + implementation +
                " fineness=" + std::to_string(f) +
                " concurrency=" +
                std::to_string(ThreadManager::instance().concurrency());
            LOGGING_PRINTLN_AT(1,context << " starting, warmups=" << warmups
                << ", samples=" << n)
            auto timings=runner.run(selected_operation,n,
                [&] {
                    GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-2_pr));
                    Evolver evolver(system,integrator);
                    analyser=std::make_unique<Analyser>(evolver);
                    manager.reset();
                    if(implementation=="robdd") {
                        manager=std::make_shared<RobddManager>();
                        analyser->set_paving_factory([owner=manager](Grid const& grid_value) {
                            return Paving(new RobddPaving(owner,grid_value));
                        });
                    } else {
                        analyser->set_paving_factory([](Grid const& grid_value) {
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
                    std::size_t const next=completed_rounds+1u;
                    bool const warmup=next<=warmups;
                    LOGGING_PRINTLN_AT(1,context << " round " << next << "/"
                        << total_rounds << (warmup ? " [warmup]" : " [sample]")
                        << " running...")
                },
                [&] {
                    result=std::make_unique<Storage>(
                        selected_operation=="lower_reach"
                            ? analyser->lower_reach(initial)
                            : analyser->outer_chain_reach(initial));
                },
                [&] {
                    if(result==nullptr || result->is_empty())
                        throw std::runtime_error("Empty reachable set");
                    Bench::keep_alive(result->size());
                    ++completed_rounds;
                    ++completed_executions;
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
                    LOGGING_PRINTLN_AT(1,context << " round " << completed_rounds
                        << "/" << total_rounds << " completed")
                });

            // Compare the final reached geometry, not the backend-specific
            // enabled-leaf counts. This conversion is OUTSIDE timed work.
            BinaryTreePaving materialized(result->state_paving().grid());
            auto const& state=result->state_paving();
            for(auto it=state.begin(),end=state.end();it!=end;++it)
                materialized.adjoin(*it);
            if(implementation=="tree") {
                tree_reference=materialized;
            } else {
                if(not tree_reference)
                    throw std::runtime_error("Tree reference must precede ROBDD for each operation/fineness");
                if(not subset(materialized,*tree_reference)
                   or not subset(*tree_reference,materialized))
                    throw std::runtime_error("ROBDD reached a different geometric set");
            }
            report.add_row({
                implementation,std::to_string(f),
                std::to_string(ThreadManager::instance().concurrency()),
                selected_operation
            },Bench::summarize(timings));
            runner.configuration_completed(config);
            LOGGING_PRINTLN_AT(1,context << " completed")
        });
        Logger::instance().release(total_scope.scope());
        report.print();
        return 0;
    }catch(std::exception const& e) {
        LOGGING_PRINTLN("benchmark_infinite_time_reachability: "<<e.what())
        return 1;
    }
}

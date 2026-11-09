// Whole-analysis infinite-time LOWER reachability benchmark.
// Workload matches tests/test_infinite_time_reachability.cpp.
// The analyser selects the backend for every intermediate Paving.
#include "ariadne-benchmarking.hpp"
#include "threading/thread_manager.hpp"
#include "threading/threading_command_line_interface.hpp"
#include "function/taylor_function.hpp"
#include "function/formula.hpp"
#include "algebra/algebra.hpp"
#include "set/function_set.hpp"
#include "evolution/vector_field_evolver.hpp"
#include "reasoning/reachability_analyser.hpp"
#include "integration/integrator.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/robdd_paving.hpp"
#include <iostream>
#include <memory>
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
        Bench::Matrix matrix;
        matrix.add(fineness).add(backend).add(samples);
        Bench::CommandLine cli;
        ThreadManager::instance();
        cli.add(threading_command_line_contribution());
        cli.add_parameters(matrix);
        if(!cli.acquire(argc,argv)) return 0;

        Bench::Report report("Infinite-time lower reachability",cli.summary_fields());
        report.set_dimension_columns({
            {"Backend"},{"Maximum grid fineness"},{"Concurrency"},{"Operation"}});
        report.set_parameter_header(matrix.header());
        Bench::Runner runner(cli,matrix.combinations());

        System system=make_system();
        RealVariable x("x"),y("y");
        RealExpressionBoundedConstraintSet symbolic_initial(
            {1.98_dec<=x<=1.99_dec,0.01_dec<=y<=0.02_dec});
        BoundedConstraintSet initial(
            symbolic_initial.euclidean_set(system.state_space()));
        BoundingDomain bounding({{-4,4},{-4,4}});

        matrix.for_each([&](Bench::Configuration const& config) {
            runner.configuration_started(config);
            unsigned f=config.value(fineness),n=config.value(samples);
            std::string const implementation=config.value(backend);
            if(implementation!="tree" and implementation!="robdd")
                throw std::invalid_argument(
                    "Unsupported backend: tree or robdd. CachedTree lacks mutable PavingInterface");
            if(n==0u || f>16u)
                throw std::invalid_argument("samples must be >0; fineness <=16");
            std::unique_ptr<Analyser> analyser;
            std::unique_ptr<Storage> result;
            std::shared_ptr<RobddManager> manager;
            auto timings=runner.run("lower_reach",n,
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
                    analyser->configuration().set_transient_time(4.0_dec);
                    analyser->configuration().set_bounding_domain_ptr(
                        std::make_shared<BoundingDomain>(bounding));
                    result.reset();
                },
                [&] {
                    result=std::make_unique<Storage>(analyser->lower_reach(initial));
                    Bench::keep_alive(result->size());
                },
                [&] {
                    if(result==nullptr || result->is_empty())
                        throw std::runtime_error("Empty reachable set");
                });
            report.add_row({
                implementation,std::to_string(f),
                std::to_string(ThreadManager::instance().concurrency()),
                "lower_reach"
            },Bench::summarize(timings));
            runner.configuration_completed(config);
        });
        report.print();
        return 0;
    }catch(std::exception const& e) {
        std::cerr<<"benchmark_infinite_time_reachability: "<<e.what()<<'\n';
        return 1;
    }
}

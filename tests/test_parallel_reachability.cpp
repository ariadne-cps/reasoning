// Exact serial/parallel regression for incremental updates to shared targets.
#include <memory>
#include <utility>

#include "ariadne-testing.hpp"
#include "evolution/vector_field_evolver.hpp"
#include "integration/integrator.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/robdd_paving.hpp"
#include "reasoning/reachability_analyser.hpp"
#include "threading/thread_manager.hpp"

using namespace Ariadne;

namespace {

struct ExposedAnalyser : ReachabilityAnalyser<VectorField> {
    using ReachabilityAnalyser<VectorField>::ReachabilityAnalyser;
    using ReachabilityAnalyser<VectorField>::_adjoin_upper_reach_evolve;
};

struct RestoreConcurrency {
    ThreadManager& manager;
    std::size_t previous;
    ~RestoreConcurrency() {
        try { manager.set_concurrency(previous); } catch(...) { }
    }
};

class TestParallelReachability : public Testing::ContextFixture {
  public:
    using Testing::ContextFixture::ContextFixture;

    Void test_shared_incremental_upper_reach_evolve() {
        auto& threading=ThreadManager::instance();
        RestoreConcurrency restore{threading,threading.concurrency()};
        if(threading.maximum_concurrency()<2u) return;

        RealVariable x("x"),y("y");
        VectorField system({dot(x)=-x,dot(y)=-y});
        GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-2_pr));
        VectorFieldEvolver evolver(system,integrator);
        ExposedAnalyser analyser(evolver);
        Grid grid(2u);
        GridCell root(grid,0u,BinaryWord{});

        for(bool use_robdd : {false,true}) {
            // Same-manager tests are valid; these are not benchmark samples.
            auto manager=std::make_shared<RobddManager>();
            auto fresh=[&]() -> LabelledAugmentedPaving {
                if(use_robdd)
                    return LabelledAugmentedPaving(
                        Paving(new RobddPaving(manager,grid)),system);
                return LabelledAugmentedPaving(
                    Paving(new BinaryTreePaving(grid)),system);
            };
            auto input=fresh();
            GridCell first=root.split(false).split(false);
            GridCell second=root.split(true).split(true);
            input.state_paving().adjoin(first);
            input.state_paving().adjoin(second);
            ARIADNE_TESTING_PROVE(input.size()>=2u);

            auto compute=[&](std::size_t concurrency) {
                threading.set_concurrency(concurrency);
                auto reach=fresh();
                auto evolved=fresh();
                // Exercise incremental coverage, not only empty targets.
                reach.state_paving().adjoin(first);
                analyser._adjoin_upper_reach_evolve(
                    reach,evolved,input,0.125_x,1u,evolver);
                return std::make_pair(std::move(reach),std::move(evolved));
            };
            auto serial=compute(0u);
            ARIADNE_TESTING_PROVE(serial.first.size()>0u);
            ARIADNE_TESTING_PROVE(serial.second.size()>0u);
            for(std::size_t concurrency : {2u,4u}) {
                if(concurrency>threading.maximum_concurrency()) continue;
                auto parallel=compute(concurrency);
                ARIADNE_TESTING_PROVE(subset(serial.first,parallel.first));
                ARIADNE_TESTING_PROVE(subset(parallel.first,serial.first));
                ARIADNE_TESTING_PROVE(subset(serial.second,parallel.second));
                ARIADNE_TESTING_PROVE(subset(parallel.second,serial.second));
            }
        }
    }
};

} // namespace

int main(int argc,char** argv) {
    ARIADNE_TESTING_SUITE("test_parallel_reachability");
    ARIADNE_TESTING_ADD_TEST(
        TestParallelReachability,test_shared_incremental_upper_reach_evolve);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc,argv);
}

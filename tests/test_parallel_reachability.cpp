// Exact serial/parallel regression for incremental updates to shared targets.
#include <memory>
#include <utility>

#include "ariadne-testing.hpp"
#include "evolution/enclosure.hpp"
#include "evolution/orbit.hpp"
#include "evolution/vector_field_evolver.hpp"
#include "evolution/iterated_map_evolver.hpp"
#include "integration/integrator.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/robdd_paving.hpp"
#include "reasoning/reachability_analyser.hpp"
#include "reasoning/parallel_reachability.hpp"
#include <array>
#include <cfenv>
#include <string>
#include "threading/thread_manager.hpp"

using namespace Ariadne;

namespace {

struct ExposedAnalyser : ReachabilityAnalyser<VectorField> {
    using ReachabilityAnalyser<VectorField>::ReachabilityAnalyser;
    using ReachabilityAnalyser<VectorField>::_adjoin_upper_reach_evolve;
};

struct ExposedMapAnalyser : ReachabilityAnalyser<IteratedMap> {
    using ReachabilityAnalyser<IteratedMap>::ReachabilityAnalyser;
    using ReachabilityAnalyser<IteratedMap>::_adjoin_upper_reach_evolve;
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

    Void test_discrete_three_cells_and_single_cell_fallback() {
        auto& threads=ThreadManager::instance();
        RestoreConcurrency restore{threads,threads.concurrency()};
        RealVariable x("x");
        IteratedMap system(List<PrimedRealAssignment>{next(x)=x/2});
        IteratedMapEvolver evolver(system);
        ExposedMapAnalyser analyser(evolver);
        Grid grid(1u);
        GridCell root(grid,2u,BinaryWord{});
        auto fresh=[&] { return LabelledAugmentedPaving(
            Paving(new BinaryTreePaving(grid)),system); };
        for(bool singleton:{true,false}) {
            auto input=fresh();
            input.state_paving().adjoin(root.split(false).split(false));
            if(not singleton) {
                input.state_paving().adjoin(root.split(false).split(true));
                input.state_paving().adjoin(root.split(true).split(false));
            }
            auto compute=[&](std::size_t workers) {
                threads.set_concurrency(workers);
                auto reach=fresh(),evolved=fresh();
                analyser._adjoin_upper_reach_evolve(reach,evolved,input,Integer(1),0u,evolver);
                return std::make_pair(reach,evolved);
            };
            auto serial=compute(0u);
            auto parallel=compute(std::min<std::size_t>(2u,threads.maximum_concurrency()));
            ARIADNE_TESTING_PROVE(not serial.first.is_empty());
            ARIADNE_TESTING_PROVE(subset(serial.first,parallel.first));
            ARIADNE_TESTING_PROVE(subset(parallel.first,serial.first));
            ARIADNE_TESTING_PROVE(subset(serial.second,parallel.second));
            ARIADNE_TESTING_PROVE(subset(parallel.second,serial.second));
        }
    }

    Void test_failure_precedence_and_complete_join() {
        auto& threading=ThreadManager::instance();
        for(unsigned failures:{0u,1u,2u,3u}) {
            std::array<unsigned,3> completed{};
            std::string error;
            try {
                ReasoningDetail::run_reachability_lanes(3u,[&](std::size_t lane) {
                    ++completed[lane];
                    if(lane==0u && (failures&1u)!=0u) throw std::runtime_error("caller");
                    if(lane==1u && (failures&2u)!=0u) throw std::runtime_error("worker");
                },threading);
            } catch(std::runtime_error const& failure) { error=failure.what(); }
            for(auto count:completed) ARIADNE_TESTING_EXPECT_EQUAL(count,1u);
            std::string expected=(failures&1u)!=0u ? "caller" : ((failures&2u)!=0u ? "worker" : "");
            ARIADNE_TESTING_EXPECT_EQUAL(error,expected);
        }
        unsigned calls=0u;
        ReasoningDetail::run_reachability_lanes(0u,[&](std::size_t) { ++calls; },threading);
        ARIADNE_TESTING_EXPECT_EQUAL(calls,0u);
        ReasoningDetail::run_reachability_lanes(1u,[&](std::size_t) { ++calls; },threading);
        ARIADNE_TESTING_EXPECT_EQUAL(calls,1u);
        for(int mode:{FE_TONEAREST,FE_DOWNWARD,FE_UPWARD,FE_TOWARDZERO})
            ARIADNE_TESTING_EXPECT_EQUAL(ReasoningDetail::require_caller_rounding_mode(mode),mode);
        ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error,ReasoningDetail::require_caller_rounding_mode(-1));
        ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error,ReasoningDetail::require_caller_rounding_mode(-2));
    }

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
            input.state_paving().adjoin(root.split(false).split(true));
            ARIADNE_TESTING_PROVE(input.size()>=2u);

            threading.set_concurrency(2u);
            auto singleton=fresh(),single_reach=fresh(),single_evolved=fresh();
            singleton.state_paving().adjoin(first);
            analyser._adjoin_upper_reach_evolve(single_reach,single_evolved,
                singleton,0.125_x,1u,evolver);
            ARIADNE_TESTING_PROVE(not single_evolved.is_empty());

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
    ARIADNE_TESTING_ADD_TEST(TestParallelReachability,test_failure_precedence_and_complete_join);
    ARIADNE_TESTING_ADD_TEST(TestParallelReachability,test_discrete_three_cells_and_single_cell_fallback);
    ARIADNE_TESTING_ADD_TEST(
        TestParallelReachability,test_shared_incremental_upper_reach_evolve);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc,argv);
}

/***************************************************************************
 *            test_hybrid_reachability.cpp
 *
 *  Copyright  2006-20  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne.
 *
 *  Ariadne is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <fstream>
#include <iostream>

#include "utility/tuple.hpp"
#include "numeric/rational.hpp"
#include "algebra/vector.hpp"
#include "algebra/matrix.hpp"
#include "function/function.hpp"
#include "geometry/box.hpp"
#include "set/list_set.hpp"
#include "integration/integrator.hpp"
#include "evolution/orbit.hpp"
#include "system/hybrid_automata.hpp"
#include "evolution/hybrid_time.hpp"
#include "hybrid/hybrid_paving.hpp"
#include "hybrid/hybrid_set.hpp"
#include "evolution/hybrid_evolver.hpp"
#include "labelled/graphics_interface.hpp"
#include "labelled/figure.hpp"
#include "hybrid/hybrid_graphics.hpp"
#include "reasoning/hybrid_reachability_analyser.hpp"
#include "reasoning/analysis_paving_factory.hpp"
#include "threading/thread_manager.hpp"
#include "drawing/drawer_manager.hpp"
#include "drawing/drawer.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;
using namespace std;

namespace Ariadne {
// Defined in src/reachability_analyser.cpp; declared here because the
// declaration lives in the .tpl.hpp, which tests do not include.
OutputStream& operator<<(OutputStream& os, const ChainOverspillPolicy& policy);
}


Colour reach_set_colour(0.25,0.25,0.50);
Colour intermediate_set_colour(0.50,0.50,0.75);
Colour final_set_colour(0.75,0.75,1.00);
Colour initial_set_colour(0.75,0.75,1.00);
Colour guard_set_colour(0.75,0.75,0.75);

// Expose the protected helper so its behaviour can be verified directly.
struct ExposedHybridAnalyser : public ReachabilityAnalyser<HybridAutomatonInterface> {
    using ReachabilityAnalyser<HybridAutomatonInterface>::ReachabilityAnalyser;
    using ReachabilityAnalyser<HybridAutomatonInterface>::_upper_reach;
    using ReachabilityAnalyser<HybridAutomatonInterface>::_adjoin_upper_reach_evolve;
};

// Test evolution of realistic hybrid systems
class TestHybridReachability : public Ariadne::Testing::ContextFixture
{
    shared_ptr<HybridReachabilityAnalyser> analyser;
  private:
    Void _set_analyser(const HybridAutomatonInterface& system);
    static HybridAutomaton _build_simple_system();
    static HybridBoundedConstraintSet _initial_set(const DiscreteLocation& q);
  public:
    using ContextFixture::ContextFixture;
    Void test_bouncing_ball();
    Void test_analyser_configuration();
    Void test_lower_reachability();
    Void test_upper_reachability();
    Void test_outer_chain_reach();
    Void test_verify_safety();
    Void test_upper_reach_helper();
    Void test_parallel_upper_reach_evolve();
    Void test_selected_robdd_backend();
};

HybridAutomaton TestHybridReachability::_build_simple_system()
{
    HybridAutomaton system;
    DiscreteLocation q;
    RealVariable x("x");
    system.new_mode(q, {dot(x)=-x});
    return system;
}

HybridBoundedConstraintSet TestHybridReachability::_initial_set(const DiscreteLocation& q)
{
    RealVariable x("x");
    return HybridBoundedConstraintSet(q, {0.9_dec<=x<=1.1_dec});
}

Void TestHybridReachability::_set_analyser(const HybridAutomatonInterface& system)
{
    GeneralHybridEvolver evolver(system);
    evolver.set_integrator(TaylorPicardIntegrator(StepMaximumError(1e-5)));
    evolver.configuration().set_maximum_step_size(0.25);
    evolver.configuration().set_maximum_enclosure_radius(0.125);
    evolver.configuration().set_maximum_enclosure_radius(0.5);
    analyser.reset(new HybridReachabilityAnalyser(evolver));
    analyser->configuration().set_maximum_grid_fineness(2);
    analyser->configuration().set_lock_to_grid_time(5);
    ARIADNE_TESTING_NOTE(analyser->configuration());
}

Void TestHybridReachability::test_bouncing_ball() {
    HybridAutomaton bouncing_ball;
    Real one(1);
    DiscreteLocation q;
    DiscreteEvent e("e");
    RealVariable x("x");
    RealVariable v("v");
    TimeVariable t;

    Real lambda(0.5_x);
    bouncing_ball.new_mode(q,{dot(x)=v,dot(v)=-one});
    bouncing_ball.new_transition(q,e,q,{next(x)=x,next(v)=-lambda*v},x<=0,EventKind::IMPACT);
    ARIADNE_TESTING_NOTE(bouncing_ball);

    Dyadic height=2;
    Dyadic radius=1/pow(two,6);
    HybridBoundedConstraintSet initial(q,{height-radius<=x<=height+radius,-radius<=v<=radius});
    // Exercise the first impact, not a long sequence of bounces.
    Decimal tmax(2.5);
    Natural maxsteps=2u;
    HybridTime time(tmax,maxsteps);

    this->_set_analyser(bouncing_ball);

    auto reach_evolve=analyser->upper_reach_evolve(initial,time);

    ARIADNE_TESTING_PROVE(reach_evolve.first.size()>0u);
    ARIADNE_TESTING_PROVE(reach_evolve.second.size()>0u);
}

Void TestHybridReachability::test_analyser_configuration() {
    HybridAutomaton system = _build_simple_system();
    DiscreteLocation q;
    RealVariable x("x");
    DiscreteEvent e("e");
    this->_set_analyser(system);

    auto& config = analyser->configuration();
    const ReachabilityAnalyser<HybridAutomatonInterface>& const_analyser = *analyser;

    config.set_lock_to_grid_events(Set<DiscreteEvent>{e});
    ARIADNE_TESTING_EXPECT_EQUAL(const_analyser.configuration().lock_to_grid_events().size(), 1u);

    HybridExactBoxes bounding;
    bounding.insert(q, system.continuous_state_space(q), ExactBoxType({{-4,4}}));
    config.set_bounding_domain_ptr(std::make_shared<HybridExactBoxes>(bounding));
    ARIADNE_TESTING_EXPECT_EQUAL(const_analyser.configuration().bounding_domain().locations().size(), 1u);
    // Print the configuration with a bounding domain set.
    ARIADNE_TESTING_NOTE(config);

    // A grid on a different hybrid space is rejected.
    HybridAutomaton other_system;
    DiscreteLocation other_q;
    RealVariable u("u");
    other_system.new_mode(other_q, {dot(u)=-u});
    ARIADNE_TESTING_EXPECT_THROWS(std::runtime_error,
        config.set_grid(std::make_shared<HybridGrid>(other_system.state_space())));

    config.set_scaling(x, ApproximateDouble(0.5));

    ARIADNE_TESTING_NOTE(const_analyser.configuration().transient_time());
    ARIADNE_TESTING_NOTE(const_analyser.configuration().lock_to_grid_steps());
    ARIADNE_TESTING_NOTE(analyser->evolver().system().name());
    ARIADNE_TESTING_NOTE(analyser->system().name());

    std::unique_ptr<HybridReachabilityAnalyser> copy(analyser->clone());
    ARIADNE_TESTING_PROVE(copy.get() != nullptr);

    ARIADNE_TESTING_NOTE(ChainOverspillPolicy::IGNORE);
    ARIADNE_TESTING_NOTE(ChainOverspillPolicy::WARNING);
    ARIADNE_TESTING_NOTE(ChainOverspillPolicy::ERROR);
    ARIADNE_TESTING_NOTE(static_cast<ChainOverspillPolicy>(255));
}

Void TestHybridReachability::test_lower_reachability() {
    HybridAutomaton system = _build_simple_system();
    DiscreteLocation q;
    this->_set_analyser(system);
    auto initial = this->_initial_set(q);
    analyser->configuration().set_maximum_grid_fineness(1);
    analyser->configuration().set_maximum_grid_extent(3u);
    analyser->configuration().set_lock_to_grid_time(0.5);
    HybridTime time(Decimal(0.5), 1u);

    auto lower_evolve = analyser->lower_evolve(initial, time);
    ARIADNE_TESTING_PROVE(lower_evolve.size() > 0u);

    auto lower_reach = analyser->lower_reach(initial, time);
    ARIADNE_TESTING_PROVE(lower_reach.size() > 0u);

    auto reach_evolve = analyser->lower_reach_evolve(initial, time);
    ARIADNE_TESTING_PROVE(reach_evolve.first.size() > 0u);
    ARIADNE_TESTING_PROVE(reach_evolve.second.size() > 0u);

    HybridExactBoxes bounding;
    bounding.insert(q, system.continuous_state_space(q), ExactBoxType({{-2,2}}));
    analyser->configuration().set_bounding_domain_ptr(std::make_shared<HybridExactBoxes>(bounding));
    analyser->configuration().set_transient_time(0.0);
    // With zero transient time no infinite-time evolution is performed.
    auto infinite_lower_reach = analyser->lower_reach(initial);
    ARIADNE_TESTING_PROVE(infinite_lower_reach.is_empty());

    analyser->configuration().set_transient_time(0.5);
    auto transient_lower_reach = analyser->lower_reach(initial);
    ARIADNE_TESTING_PROVE(transient_lower_reach.size() > 0u);

    // No bounding domain: the restriction steps are skipped.
    analyser->configuration().set_bounding_domain_ptr(nullptr);
    analyser->lower_reach(initial);
}

Void TestHybridReachability::test_upper_reachability() {
    HybridAutomaton system = _build_simple_system();
    DiscreteLocation q;
    this->_set_analyser(system);
    auto initial = this->_initial_set(q);
    analyser->configuration().set_maximum_grid_fineness(2);
    HybridTime time(Decimal(1.0), 2u);

    auto upper_evolve = analyser->upper_evolve(initial, time);
    ARIADNE_TESTING_PROVE(upper_evolve.size() > 0u);

    auto upper_reach = analyser->upper_reach(initial, time);
    ARIADNE_TESTING_PROVE(upper_reach.size() > 0u);

    // A time that is not a multiple of lock_to_grid_time exercises the
    // remainder-time step of upper_reach_evolve.
    auto reach_evolve = analyser->upper_reach_evolve(initial, HybridTime(Decimal(1.2), 1u));
    ARIADNE_TESTING_PROVE(reach_evolve.first.size() > 0u);
    ARIADNE_TESTING_PROVE(reach_evolve.second.size() > 0u);
}

Void TestHybridReachability::test_outer_chain_reach() {
    HybridAutomaton system = _build_simple_system();
    DiscreteLocation q;
    this->_set_analyser(system);
    auto initial = this->_initial_set(q);
    analyser->configuration().set_maximum_grid_fineness(2);
    analyser->configuration().set_transient_time(0.0);
    HybridExactBoxes bounding;
    bounding.insert(q, system.continuous_state_space(q), ExactBoxType({{-4,4}}));
    analyser->configuration().set_bounding_domain_ptr(std::make_shared<HybridExactBoxes>(bounding));

    auto chain_reach = analyser->outer_chain_reach(initial);
    ARIADNE_TESTING_PROVE(chain_reach.size() > 0u);

    analyser->configuration().set_transient_time(1.0);
    auto transient_chain_reach = analyser->outer_chain_reach(initial);
    ARIADNE_TESTING_PROVE(transient_chain_reach.size() > 0u);

    // No bounding domain: the restriction steps are skipped.
    analyser->configuration().set_bounding_domain_ptr(nullptr);
    auto unbounded_chain_reach = analyser->outer_chain_reach(initial);
    ARIADNE_TESTING_PROVE(unbounded_chain_reach.size() > 0u);
}

Void TestHybridReachability::test_verify_safety() {
    HybridAutomaton system = _build_simple_system();
    DiscreteLocation q;
    RealVariable x("x");
    this->_set_analyser(system);
    auto initial = this->_initial_set(q);
    HybridBoundedConstraintSet safe(q, {-3_dec<=x<=3_dec});
    analyser->configuration().set_maximum_grid_fineness(2);
    analyser->configuration().set_transient_time(0.0);
    analyser->configuration().set_bounding_domain_ptr(nullptr);

    auto certificate = analyser->verify_safety(initial, safe);
    ARIADNE_TESTING_PROVE(definitely(certificate.is_safe));
    ARIADNE_TESTING_PROVE(certificate.chain_reach_set.size() > 0u);

    analyser->configuration().set_transient_time(1.0);
    auto transient_certificate = analyser->verify_safety(initial, safe);
    ARIADNE_TESTING_PROVE(definitely(transient_certificate.is_safe));

    HybridBoundedConstraintSet outside_safe(q, {2_dec<=x<=3_dec});
    auto negative_certificate = analyser->verify_safety(initial, outside_safe);
    ARIADNE_TESTING_PROVE(is_indeterminate(negative_certificate.is_safe));
}


Void TestHybridReachability::test_selected_robdd_backend() {
    HybridAutomaton system=_build_simple_system();
    DiscreteLocation q;
    GeneralHybridEvolver evolver(system);
    evolver.set_integrator(TaylorPicardIntegrator(StepMaximumError(1e-5)));
    ExposedHybridAnalyser exposed_analyser(evolver);
    exposed_analyser.configuration().set_maximum_grid_fineness(1u);
    auto initial=_initial_set(q);
    HybridTime time(Decimal(0.25),1u);

    auto tree=exposed_analyser.upper_reach(initial,time);
    ARIADNE_TESTING_PROVE(tree.size()>0u);

    auto manager=std::make_shared<RobddManager>();
    exposed_analyser.set_paving_factory([manager](Grid const& grid) {
        return Paving(new RobddPaving(manager,grid));
    });
    auto robdd=exposed_analyser.upper_reach(initial,time);
    ARIADNE_TESTING_PROVE(robdd.size()>0u);
    ARIADNE_TESTING_PROVE(
        dynamic_cast<RobddPaving const*>(
            &static_cast<PavingInterface const&>(robdd.state_paving()[q]))!=nullptr);
    // Compare complete analysis results geometrically. Algebra between
    // different backends is intentionally unsupported; GridCell coverage
    // is independent of both the representation and its ROBDD manager.
    auto geometrically_contained=[](HybridAugmentedPaving const& candidate,
                                   HybridAugmentedPaving const& container) {
        for(auto it=candidate.begin();it!=candidate.end();++it) {
            auto const cell=*it;
            if(!container.state_paving().nontrivial_location(cell.location()) ||
               !container.state_paving()[cell.location()].superset(cell.euclidean_set()))
                return false;
        }
        return true;
    };
    ARIADNE_TESTING_PROVE(geometrically_contained(tree,robdd));
    ARIADNE_TESTING_PROVE(geometrically_contained(robdd,tree));
}

Void TestHybridReachability::test_parallel_upper_reach_evolve() {
    // Verify the same templated cell scheduler on hybrid storage, including
    // concurrent first insertion of a location in the destination map.
    auto& threads=ThreadManager::instance();
    auto const previous=threads.concurrency();
    struct Restore {
        ThreadManager& manager;
        std::size_t saved;
        ~Restore() { try { manager.set_concurrency(saved); } catch(...) { } }
    } restore{threads,previous};
    if(threads.maximum_concurrency()<2u) return;

    HybridAutomaton system=_build_simple_system();
    DiscreteLocation q;
    GeneralHybridEvolver evolver(system);
    evolver.set_integrator(TaylorPicardIntegrator(StepMaximumError(1e-5)));
    ExposedHybridAnalyser exposed_analyser(evolver);
    auto const grid=exposed_analyser.configuration().grid();
    auto fresh=[&] {
        return make_analysis_paving<HybridAugmentedPaving>(grid,system);
    };

    HybridAugmentedPaving input=fresh();
    GridCell root(grid[q],0u,BinaryWord{});
    HybridGridCell first(q,system.continuous_state_space(q),
                         root.split(false).split(false));
    HybridGridCell second(q,system.continuous_state_space(q),
                          root.split(true).split(true));
    input.state_paving().adjoin(first);
    input.state_paving().adjoin(second);
    ARIADNE_TESTING_PROVE(input.size()>=2u);

    auto compute=[&](std::size_t concurrency) {
        threads.set_concurrency(concurrency);
        HybridAugmentedPaving reach=fresh();
        HybridAugmentedPaving evolved=fresh();
        // The reach destination already contains a cell; the evolve
        // destination is empty and must create its location during adjoin.
        reach.state_paving().adjoin(first);
        exposed_analyser._adjoin_upper_reach_evolve(reach,evolved,input,
            HybridTime(Decimal(0.25),1u),1u,evolver);
        return std::make_pair(std::move(reach),std::move(evolved));
    };
    auto serial=compute(0u);
    ARIADNE_TESTING_PROVE(serial.first.size()>0u);
    ARIADNE_TESTING_PROVE(serial.second.size()>0u);
    for(std::size_t workers : {2u,4u}) {
        if(workers>threads.maximum_concurrency()) continue;
        auto parallel=compute(workers);
        ARIADNE_TESTING_PROVE(subset(serial.first,parallel.first));
        ARIADNE_TESTING_PROVE(subset(parallel.first,serial.first));
        ARIADNE_TESTING_PROVE(subset(serial.second,parallel.second));
        ARIADNE_TESTING_PROVE(subset(parallel.second,serial.second));
    }
}

Void TestHybridReachability::test_upper_reach_helper() {
    HybridAutomaton system = _build_simple_system();
    DiscreteLocation q;
    auto initial = this->_initial_set(q);
    GeneralHybridEvolver evolver(system);
    evolver.set_integrator(TaylorPicardIntegrator(StepMaximumError(1e-5)));
    ExposedHybridAnalyser exposed_analyser(evolver);
    exposed_analyser.configuration().set_maximum_grid_fineness(2);
    HybridTime time(Decimal(1.0), 2u);

    auto reach = exposed_analyser.upper_reach(initial, time);
    ARIADNE_TESTING_PROVE(reach.size() > 0u);
    auto upper = exposed_analyser._upper_reach(reach, time, 2u, evolver);
    ARIADNE_TESTING_PROVE(upper.size() > 0u);

    // clone() on the base class (not the HybridReachabilityAnalyser override).
    std::unique_ptr<ReachabilityAnalyser<HybridAutomatonInterface>> base_copy(exposed_analyser.clone());
    ARIADNE_TESTING_PROVE(base_copy.get() != nullptr);
}


int main(int argc, char** argv)
{
    DrawerManager::instance().set_drawer(GridDrawer(2));

    ARIADNE_TESTING_SUITE("test_hybrid_reachability");
    ARIADNE_TESTING_ADD_TEST(TestHybridReachability, test_bouncing_ball);
    ARIADNE_TESTING_ADD_TEST(TestHybridReachability, test_analyser_configuration);
    ARIADNE_TESTING_ADD_TEST(TestHybridReachability, test_lower_reachability);
    ARIADNE_TESTING_ADD_TEST(TestHybridReachability, test_upper_reachability);
    ARIADNE_TESTING_ADD_TEST(TestHybridReachability, test_outer_chain_reach);
    ARIADNE_TESTING_ADD_TEST(TestHybridReachability, test_verify_safety);
    ARIADNE_TESTING_ADD_TEST(TestHybridReachability, test_upper_reach_helper);
    ARIADNE_TESTING_ADD_TEST(TestHybridReachability, test_parallel_upper_reach_evolve);
    ARIADNE_TESTING_ADD_TEST(TestHybridReachability, test_selected_robdd_backend);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}

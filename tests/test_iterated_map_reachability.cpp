/***************************************************************************
 *            test_iterated_map_reachability.cpp
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

#include <iostream>
#include <memory>

#include "function/formula.hpp"
#include "algebra/algebra.hpp"
#include "set/function_set.hpp"
#include "paving/binary_tree_paving.hpp"
#include "evolution/orbit.hpp"
#include "evolution/iterated_map_evolver.hpp"
#include "expression-set/expression_set.hpp"
#include "system/iterated_map.hpp"
#include "logging/logging.hpp"

#include "reasoning/reachability_analyser.hpp"
#include "paving/augmented_paving.hpp"

#include "ariadne-testing.hpp"

using namespace Ariadne;
using namespace std;


// Expose the protected helper so its behaviour can be verified directly.
struct ExposedIteratedMapAnalyser : public ReachabilityAnalyser<IteratedMap> {
    using ReachabilityAnalyser<IteratedMap>::ReachabilityAnalyser;
    using ReachabilityAnalyser<IteratedMap>::_upper_reach;
};

// Restore the logging verbosity on destruction, so verbose diagnostics do
// not leak into the other tests.
struct LogVerbosityScope {
    unsigned _previous;
    explicit LogVerbosityScope(unsigned verbosity)
        : _previous(Logger::instance().configuration().verbosity()) {
        Logger::instance().configuration().set_verbosity(verbosity);
    }
    ~LogVerbosityScope() { Logger::instance().configuration().set_verbosity(_previous); }
};


class TestIteratedMapReachability : public Ariadne::Testing::ContextFixture
{
  public:
    typedef IteratedMap SystemType;
    typedef IteratedMapEvolver EvolverType;
    typedef SystemType::TimeType TimeType;
    typedef RealExpressionBoundedConstraintSet SymbolicSetType;
    typedef BoundedConstraintSet SetType;
    typedef ReachabilityAnalyser<SystemType> AnalyserType;
    typedef AnalyserType::BoundingDomainType BoundingDomainType;
    typedef AnalyserType::StorageType StorageType;

    RealVariable x;
    RealVariable y;
    SystemType system;
    ExposedIteratedMapAnalyser analyser;
    SymbolicSetType symbolic_initial_set;
    SetType initial_set;
    SymbolicSetType symbolic_safe_set;
    SetType safe_set;

    static SystemType build_system()
    {
        RealVariable x("x"), y("y");
        // Contracting map, so chain-reachability computations terminate.
        return SystemType({next(x)=x/2, next(y)=y/2});
    }

    TestIteratedMapReachability(Ariadne::Testing::Context& context)
        : ContextFixture(context),
          x("x"), y("y"),
          system(build_system()),
          analyser(EvolverType(system)),
          symbolic_initial_set({0.9_dec<=x<=1.1_dec, 0.9_dec<=y<=1.1_dec}),
          initial_set(symbolic_initial_set.euclidean_set(system.state_space())),
          symbolic_safe_set({-2_dec<=x<=2_dec, -2_dec<=y<=2_dec}),
          safe_set(symbolic_safe_set.euclidean_set(system.state_space()))
    {
        analyser.configuration().set_maximum_grid_fineness(2);
    }

    Void test_analyser_configuration() {
        auto& config = analyser.configuration();
        const AnalyserType& const_analyser = analyser;
        const auto& const_config = const_analyser.configuration();

        // Print the configuration with no bounding domain set.
        ARIADNE_TESTING_NOTE(config);

        config.set_transient_time(Integer(2));
        ARIADNE_TESTING_EXPECT_EQUAL(const_config.transient_time(), Integer(2));
        config.set_lock_to_grid_time(Integer(2));
        ARIADNE_TESTING_EXPECT_EQUAL(const_config.lock_to_grid_time(), Integer(2));
        config.set_maximum_grid_fineness(3);
        ARIADNE_TESTING_EXPECT_EQUAL(const_config.maximum_grid_fineness(), 3u);
        config.set_maximum_grid_extent(8);
        ARIADNE_TESTING_EXPECT_EQUAL(const_config.maximum_grid_extent(), 8u);
        config.set_bounding_domain(BoundingDomainType({{-4,4},{-4,4}}));
        ARIADNE_TESTING_PROVE(const_config.bounding_domain() == BoundingDomainType({{-4,4},{-4,4}}));
        config.set_bounding_domain_ptr(std::make_shared<BoundingDomainType>(BoundingDomainType({{-4,4},{-4,4}})));
        ARIADNE_TESTING_PROVE(const_config.bounding_domain_ptr() != nullptr);
        config.set_grid(LabelledGrid(Grid(2), system.state_space()));
        ARIADNE_TESTING_PROVE(const_config.grid().space() == system.state_space());
        config.set_outer_overspill_policy(ChainOverspillPolicy::WARNING);
        ARIADNE_TESTING_PROVE(const_config.outer_overspill_policy() == ChainOverspillPolicy::WARNING);
        config.set_outer_overspill_policy(ChainOverspillPolicy::ERROR);

        ARIADNE_TESTING_NOTE(analyser.evolver().system());
        ARIADNE_TESTING_PROVE(analyser.system().state_space() == system.state_space());
        ARIADNE_TESTING_NOTE(config);

        std::unique_ptr<AnalyserType> copy(analyser.clone());
        ARIADNE_TESTING_PROVE(copy.get() != nullptr);
        ARIADNE_TESTING_PROVE(copy->system().state_space() == system.state_space());

        // Labelled views of an iterated-map system.
        LabelledGrid lgrid(system);
        LabelledBinaryTreePaving lgtp(lgrid);
        LabelledAugmentedPaving ls1(lgrid, system);
        LabelledAugmentedPaving ls2(lgtp, system);
        ARIADNE_TESTING_PROVE(ls1.state_space() == system.state_space());
        ARIADNE_TESTING_PROVE(ls2.state_space() == system.state_space());
    }

    Void test_lower_reachability() {
        auto lower_evolve = analyser.lower_evolve(initial_set, Integer(3));
        ARIADNE_TESTING_PROVE(lower_evolve.size() > 0u);

        auto lower_reach = analyser.lower_reach(initial_set, Integer(3));
        ARIADNE_TESTING_PROVE(lower_reach.size() > 0u);

        auto reach_evolve = analyser.lower_reach_evolve(initial_set, Integer(3));
        ARIADNE_TESTING_PROVE(reach_evolve.first.size() > 0u);
        ARIADNE_TESTING_PROVE(reach_evolve.second.size() > 0u);

        analyser.configuration().set_transient_time(Integer(0));
        analyser.configuration().set_bounding_domain_ptr(std::make_shared<BoundingDomainType>(BoundingDomainType({{-4,4},{-4,4}})));
        // With zero transient time no infinite-time evolution is performed.
        auto infinite_lower_reach = analyser.lower_reach(initial_set);
        ARIADNE_TESTING_PROVE(infinite_lower_reach.is_empty());

        analyser.configuration().set_transient_time(Integer(2));
        auto transient_lower_reach = analyser.lower_reach(initial_set);
        ARIADNE_TESTING_PROVE(transient_lower_reach.size() > 0u);

        // No bounding domain: the restriction steps are skipped.
        analyser.configuration().set_bounding_domain_ptr(nullptr);
        analyser.lower_reach(initial_set);
    }

    Void test_upper_reachability() {
        analyser.configuration().set_transient_time(Integer(0));
        analyser.configuration().set_bounding_domain_ptr(std::make_shared<BoundingDomainType>(BoundingDomainType({{-4,4},{-4,4}})));

        auto upper_evolve = analyser.upper_evolve(initial_set, Integer(3));
        ARIADNE_TESTING_PROVE(upper_evolve.size() > 0u);

        auto upper_reach = analyser.upper_reach(initial_set, Integer(3));
        ARIADNE_TESTING_PROVE(upper_reach.size() > 0u);

        auto reach_evolve = analyser.upper_reach_evolve(initial_set, Integer(3));
        ARIADNE_TESTING_PROVE(reach_evolve.first.size() > 0u);
        ARIADNE_TESTING_PROVE(reach_evolve.second.size() > 0u);

        auto chain_reach = analyser.outer_chain_reach(initial_set);
        ARIADNE_TESTING_PROVE(chain_reach.size() > 0u);

        analyser.configuration().set_transient_time(Integer(2));
        auto transient_chain_reach = analyser.outer_chain_reach(initial_set);
        ARIADNE_TESTING_PROVE(transient_chain_reach.size() > 0u);

        // A step count that is not a multiple of lock_to_grid_time exercises
        // the remainder-time step of upper_reach_evolve.
        analyser.configuration().set_lock_to_grid_time(Integer(2));
        auto remainder_reach_evolve = analyser.upper_reach_evolve(initial_set, Integer(5));
        ARIADNE_TESTING_PROVE(remainder_reach_evolve.first.size() > 0u);
        ARIADNE_TESTING_PROVE(remainder_reach_evolve.second.size() > 0u);

        // No bounding domain: the restriction steps are skipped.
        analyser.configuration().set_bounding_domain_ptr(nullptr);
        auto unbounded_chain_reach = analyser.outer_chain_reach(initial_set);
        ARIADNE_TESTING_PROVE(unbounded_chain_reach.size() > 0u);
    }

    Void test_upper_reach_break() {
        // With premature termination the evolved cells become empty, so the
        // time-step loop of upper_reach exits via the break.
        EvolverType evolver(system);
        evolver.configuration().set_enable_premature_termination(true);
        evolver.configuration().set_maximum_enclosure_radius(0.5);
        ExposedIteratedMapAnalyser premature_analyser(evolver);
        premature_analyser.configuration().set_maximum_grid_fineness(2);
        premature_analyser.configuration().set_lock_to_grid_time(Integer(1));
        auto upper_reach = premature_analyser.upper_reach(initial_set, Integer(4));
        ARIADNE_TESTING_NOTE(upper_reach.size());
    }

    Void test_upper_reach_helper() {
        auto reach = analyser.upper_reach(initial_set, Integer(2));
        auto upper = analyser._upper_reach(reach, Integer(2), 3u, analyser.evolver());
        ARIADNE_TESTING_PROVE(upper.size() > 0u);
    }

    Void test_verify_safety() {
        analyser.configuration().set_transient_time(Integer(0));
        analyser.configuration().set_bounding_domain_ptr(nullptr);
        auto certificate = analyser.verify_safety(initial_set, safe_set);
        ARIADNE_TESTING_PROVE(definitely(certificate.is_safe));
        ARIADNE_TESTING_PROVE(certificate.chain_reach_set.size() > 0u);

        SymbolicSetType outside_symbolic_safe_set({2_dec<=x<=3_dec, 2_dec<=y<=3_dec});
        auto outside_safe_set = outside_symbolic_safe_set.euclidean_set(system.state_space());
        auto negative_certificate = analyser.verify_safety(initial_set, outside_safe_set);
        ARIADNE_TESTING_PROVE(is_indeterminate(negative_certificate.is_safe));
    }

    Void test_verbose_logging() {
        // With high verbosity the LOGGING_* branches in the analyser are
        // all executed.
        LogVerbosityScope diagnostics(100u);
        analyser.configuration().set_transient_time(Integer(0));
        analyser.configuration().set_bounding_domain_ptr(nullptr);
        ARIADNE_TESTING_PROVE(analyser.lower_reach(initial_set, Integer(2)).size() > 0u);
        ARIADNE_TESTING_PROVE(analyser.lower_reach_evolve(initial_set, Integer(2)).first.size() > 0u);
        ARIADNE_TESTING_PROVE(analyser.upper_reach(initial_set, Integer(2)).size() > 0u);
        ARIADNE_TESTING_PROVE(analyser.upper_reach_evolve(initial_set, Integer(2)).first.size() > 0u);
        ARIADNE_TESTING_PROVE(analyser.outer_chain_reach(initial_set).size() > 0u);
        // Zero transient time: no infinite-time evolution is performed.
        ARIADNE_TESTING_PROVE(analyser.lower_reach(initial_set).is_empty());
        auto certificate = analyser.verify_safety(initial_set, safe_set);
        ARIADNE_TESTING_PROVE(definitely(certificate.is_safe));
    }

    Void test_verify_safety_reach_escapes() {
        // The initial set is inside the safe set, but the expanding map makes
        // the reachable set leave it during the recurrent stages.
        RealVariable ex("x"), ey("y");
        IteratedMap expanding({next(ex)=2*ex, next(ey)=2*ey});
        EvolverType evolver(expanding);
        ExposedIteratedMapAnalyser escaping_analyser(evolver);
        escaping_analyser.configuration().set_maximum_grid_fineness(2);
        escaping_analyser.configuration().set_transient_time(Integer(0));
        escaping_analyser.configuration().set_bounding_domain_ptr(nullptr);

        SymbolicSetType symbolic_initial({0.1_dec<=ex<=0.2_dec, 0.1_dec<=ey<=0.2_dec});
        auto initial = symbolic_initial.euclidean_set(expanding.state_space());
        SymbolicSetType symbolic_safe({-0.5_dec<=ex<=0.5_dec, -0.5_dec<=ey<=0.5_dec});
        auto safe = symbolic_safe.euclidean_set(expanding.state_space());

        auto certificate = escaping_analyser.verify_safety(initial, safe);
        ARIADNE_TESTING_PROVE(is_indeterminate(certificate.is_safe));
    }
};


int main(int argc, char** argv)
{
    ARIADNE_TESTING_SUITE("test_iterated_map_reachability");
    ARIADNE_TESTING_ADD_TEST(TestIteratedMapReachability, test_analyser_configuration);
    ARIADNE_TESTING_ADD_TEST(TestIteratedMapReachability, test_lower_reachability);
    ARIADNE_TESTING_ADD_TEST(TestIteratedMapReachability, test_upper_reachability);
    ARIADNE_TESTING_ADD_TEST(TestIteratedMapReachability, test_upper_reach_helper);
    ARIADNE_TESTING_ADD_TEST(TestIteratedMapReachability, test_verify_safety);
    ARIADNE_TESTING_ADD_TEST(TestIteratedMapReachability, test_upper_reach_break);
    ARIADNE_TESTING_ADD_TEST(TestIteratedMapReachability, test_verify_safety_reach_escapes);
    ARIADNE_TESTING_ADD_TEST(TestIteratedMapReachability, test_verbose_logging);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}

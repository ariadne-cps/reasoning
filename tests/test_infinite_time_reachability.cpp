/***************************************************************************
 *            test_infinite_time_reachability.cpp
 *
 *  Copyright  2006-20  Pieter Collins
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

/** @file tests/test_infinite_time_reachability.cpp
 *  @brief Test infinite time reachability semantics and regressions.
 */

#include <fstream>
#include <iostream>
#include <string>

#include "algebra/algebra.hpp"
#include "evolution/enclosure.hpp"
#include "evolution/orbit.hpp"
#include "evolution/vector_field_evolver.hpp"
#include "function/formula.hpp"
#include "function/taylor_function.hpp"
#include "integration/integrator.hpp"
#include "labelled/figure.hpp"
#include "labelled/labelled_augmented_paving_draw.hpp"
#include "paving/binary_tree_paving.hpp"
#include "reasoning/reachability_analyser.hpp"
#include "set/function_set.hpp"

#include "ariadne-testing.hpp"
#include "expression-set/expression_set.hpp"

using namespace std;
using namespace Ariadne;

Colour reach_set_colour(0.25, 0.25, 0.50);
Colour intermediate_set_colour(0.50, 0.50, 0.75);
Colour final_set_colour(0.75, 0.75, 1.00);
Colour evolve_set_colour(0.75, 0.75, 1.00);
Colour initial_set_colour(0.75, 0.75, 1.00);
Colour guard_set_colour(0.75, 0.75, 0.75);
Colour bounding_box_colour(0.75, 0.75, 0.875);
Colour safe_set_colour(0.50, 1.00, 0.50);

class TestInfiniteTimeReachability : public Ariadne::Testing::ContextFixture {
    typedef VectorFieldEvolver EvolverType;
    typedef EvolverType::SystemType SystemType;
    typedef SystemType::TimeType TimeType;
    typedef SystemType::StateSpaceType StateSpaceType;
    typedef EvolverType::EnclosureType EnclosureType;
    typedef RealExpressionBoundedConstraintSet SymbolicSetType;
    typedef BoundedConstraintSet SetType;
    typedef ReachabilityAnalyser<SystemType> AnalyserType;
    typedef AnalyserType::BoundingDomainType BoundingDomainType;
    typedef AnalyserType::StorageType StorageType;

    SystemType system;
    AnalyserType analyser;
    Grid grid;
    RealVariable x;
    RealVariable y;
    ExactBoxType graphics_box;
    BoundingDomainType bounding;
    SymbolicSetType symbolic_initial_set;
    SetType initial_set;
    SymbolicSetType symbolic_safe_set;
    SetType safe_set;
    TimeType reach_time;

  public:
    static SystemType build_system() {
        std::cout << std::setprecision(20);
        std::cerr << std::setprecision(20);
        std::clog << std::setprecision(20);
        RealVariable x("x");
        RealVariable y("y");
        Real a(-0.5_x);
        Real b(1.0_x);
        SystemType sys({dot(x) = -a * x - b * y, dot(y) = b * x + 2 * a * y});
        cout << "Done building system\n";
        return sys;
    }

    static AnalyserType build_analyser(const SystemType& system) {
        GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-2_pr));

        EvolverType evolver(system, integrator);

        AnalyserType analyser(evolver);
        analyser.configuration().set_maximum_grid_fineness(3);
        cout << "Done building analyser\n";
        return analyser;
    }

    TestInfiniteTimeReachability(Ariadne::Testing::Context& context)
        : ContextFixture(context),
          system(build_system()),
          analyser(build_analyser(system)),
          grid(2),
          x("x"),
          y("y"),
          graphics_box({
              {-3, 3},
              {-3, 3}
    }),
          bounding({{-4, 4}, {-4, 4}}),
          symbolic_initial_set({1.98_dec <= x <= 1.99_dec, 0.01_dec <= y <= 0.02_dec}),
          initial_set(symbolic_initial_set.euclidean_set(system.state_space())),
          symbolic_safe_set({-2_dec <= x <= 3_dec, -2_dec <= y <= 3_dec}, {sqr(x) + sqr(y) <= sqr((Real)3)}),
          safe_set(symbolic_safe_set.euclidean_set(system.state_space())),
          reach_time(3.0_x) {
        cout << "Done creating initial and safe sets\n"
             << endl;

        cout << "system=" << system << endl;
        cout << "initial_set=" << initial_set << endl;
        cout << "safe_set=" << safe_set << endl;
    }

    Void test_infinite_time_lower_reach() {
        analyser.configuration().set_transient_time(4.0_dec);
        analyser.configuration().set_bounding_domain_ptr(shared_ptr<BoundingDomainType>(new BoundingDomainType(bounding)));
        cout << analyser.configuration();

        cout << "Computing infinite time lower reachable set" << endl;
        StorageType lower_reach_set = analyser.lower_reach(initial_set);

        ARIADNE_TESTING_PROVE(lower_reach_set.size() > 0);

        cout << "Reached " << lower_reach_set.size() << " cells " << endl
             << endl;

        plot("test_reachability_analyser-map_infinite_time_lower_reach.png", Projection2d(2, 0, 1), graphics_box,
             reach_set_colour, lower_reach_set);

        // Zero transient time and no bounding domain: both restriction and
        // transient steps are skipped.
        analyser.configuration().set_transient_time(0.0_dec);
        analyser.configuration().set_bounding_domain_ptr(nullptr);
        analyser.lower_reach(initial_set);
    }

    Void test_outer_chain_reach() {
        cout << "Computing outer chain reachable set" << endl;
        analyser.configuration().set_transient_time(12.0_dec);
        analyser.configuration().set_lock_to_grid_time(6.0_dec);
        analyser.configuration().set_maximum_grid_fineness(3);
        analyser.configuration().set_bounding_domain_ptr(shared_ptr<BoundingDomainType>(new BoundingDomainType(bounding)));
        cout << analyser.configuration();

        StorageType outer_chain_reach_set = analyser.outer_chain_reach(initial_set);

        ARIADNE_TESTING_PROVE(outer_chain_reach_set.size() > 0);

        cout << "Reached " << outer_chain_reach_set.size() << " cells " << endl
             << endl;

        plot("test_reachability_analyser-map_outer_chain_reach.png", Projection2d(2, 0, 1), graphics_box,
             reach_set_colour, outer_chain_reach_set);

        cout << "Recomputing with tight restriction" << endl;

        analyser.configuration().set_maximum_grid_extent(1);
        ARIADNE_TESTING_EXPECT_THROWS(OuterChainOverspill, analyser.outer_chain_reach(initial_set));
    }

    Void test_outer_chain_reach_overspill_policies() {
        analyser.configuration().set_transient_time(12.0_dec);
        analyser.configuration().set_lock_to_grid_time(6.0_dec);
        analyser.configuration().set_maximum_grid_fineness(3);
        analyser.configuration().set_bounding_domain_ptr(shared_ptr<BoundingDomainType>(new BoundingDomainType(bounding)));
        // A tight extent forces a restriction of the computed chain reach.
        analyser.configuration().set_maximum_grid_extent(1);

        analyser.configuration().set_outer_overspill_policy(ChainOverspillPolicy::WARNING);
        auto warned_set = analyser.outer_chain_reach(initial_set);
        ARIADNE_TESTING_NOTE(warned_set.size());

        analyser.configuration().set_outer_overspill_policy(ChainOverspillPolicy::IGNORE);
        auto ignored_set = analyser.outer_chain_reach(initial_set);
        ARIADNE_TESTING_NOTE(ignored_set.size());
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_infinite_time_reachability");
    ARIADNE_TESTING_ADD_TEST(TestInfiniteTimeReachability, test_infinite_time_lower_reach);
    ARIADNE_TESTING_ADD_TEST(TestInfiniteTimeReachability, test_outer_chain_reach);
    ARIADNE_TESTING_ADD_TEST(TestInfiniteTimeReachability, test_outer_chain_reach_overspill_policies);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}

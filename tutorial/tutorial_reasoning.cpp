/***************************************************************************
 *            tutorial_reasoning.cpp
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

/** @file tutorial/tutorial_reasoning.cpp
 *  @brief Demonstrate reachability and safety analysis with Ariadne Reasoning.
 */

#include <iostream>
#include <stdexcept>

#include "integration/integrator.hpp"

#include "ariadne-reasoning.hpp"

using namespace Ariadne;

int main() {
    RealVariable x("x");
    // Contract toward the centre of a grid cell so that the chain reach
    // stabilises quickly instead of continually adding neighbouring cells.
    VectorField field(List<DottedRealAssignment>{dot(x) = 0.375_dec - x});
    GradedTaylorSeriesIntegrator integrator(StepMaximumError(1e-2_pr));
    VectorFieldEvolver evolver(field, integrator);

    ContinuousReachabilityAnalyser analyser(evolver);
    analyser.configuration().set_maximum_grid_fineness(2);

    RealExpressionBoundedConstraintSet initial_constraints({0.3125_dec <= x <= 0.34375_dec});
    RealExpressionBoundedConstraintSet safe_constraints({-2_dec <= x <= 2_dec});
    auto initial = initial_constraints.euclidean_set(field.state_space());
    auto safe = safe_constraints.euclidean_set(field.state_space());

    // Unlike finite-time upper_reach, verify_safety attempts to prove
    // that every reachable state remains in the safe set for all time.
    auto certificate = analyser.verify_safety(initial, safe);
    if (!definitely(certificate.is_safe)) {
        throw std::runtime_error("Safety could not be verified");
    }

    std::cout << "Safety verified for all time.\n"
              << "Chain-reachable grid cells: " << certificate.chain_reach_set.size() << '\n';
    return 0;
}

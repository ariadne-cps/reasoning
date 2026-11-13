/***************************************************************************
 *            hybrid_reachability_analyser.cpp
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

/** @file src/hybrid_reachability_analyser.cpp
 *  @brief Implement hybrid reachability analysis.
 */

#include "reasoning/hybrid_reachability_analyser.hpp"

#include <algorithm>
#include <list>
#include <set>
#include <sstream>
#include <string>
#include <valarray>
#include <vector>

#include "algebra/matrix.hpp"
#include "algebra/vector.hpp"
#include "evolution/hybrid_evolver.hpp"
#include "evolution/hybrid_orbit.hpp"
#include "evolution/hybrid_time.hpp"
#include "function/functional.hpp"
#include "geometry/box.hpp"
#include "hybrid/hybrid_graphics.hpp"
#include "hybrid/hybrid_paving.hpp"
#include "hybrid/hybrid_set.hpp"
#include "hybrid/hybrid_space.hpp"
#include "integration/integrator.hpp"
#include "logging/logging.hpp"
#include "numeric/numeric.hpp"
#include "paving/binary_tree_paving.hpp"
#include "reasoning/reachability_analyser.tpl.hpp"
#include "rendering/figure.hpp"
#include "set/list_set.hpp"
#include "system/hybrid_automaton_interface.hpp"
#include "utility/exceptions.hpp"

#include "equation-solving/equation_solver.hpp"
#include "linear-programming/linear_programming.hpp"

namespace Ariadne {

inline Natural compute_time_steps(HybridTime time, HybridTime lock_to_grid_time) {
    return ReasoningDetail::whole_time_steps(time.continuous_time(), lock_to_grid_time.continuous_time());
}

} // namespace Ariadne

namespace Ariadne {

template class ReachabilityAnalyser<HybridAutomatonInterface>;

HybridReachabilityAnalyser::
    HybridReachabilityAnalyser(const HybridEvolverInterface& evolver)
    : ReachabilityAnalyser<HybridAutomatonInterface>(evolver) {
}

HybridReachabilityAnalyser*
    HybridReachabilityAnalyser::
        clone() const {
    return new HybridReachabilityAnalyser(*this);
}

HybridReachabilityAnalyserConfiguration::ReachabilityAnalyserConfiguration(ReachabilityAnalyser<HybridAutomatonInterface>& analyser)
    : _analyser(analyser), _paver(new SubdivisionPaver()) {
    set_transient_time(0.0);
    set_transient_steps(0);
    set_lock_to_grid_time(1.0);
    set_lock_to_grid_steps(1);
    set_maximum_grid_fineness(3);
    set_maximum_grid_extent(16);
    set_grid(std::shared_ptr<HybridGrid>(new HybridGrid(_analyser.system().state_space(), SimpleHybridScalings())));
    set_outer_overspill_policy(ChainOverspillPolicy::ERROR);
}

OutputStream&
    HybridReachabilityAnalyserConfiguration::_write(OutputStream& os) const {
    os << "HybridReachabilityAnalyserSettings"
       << "(\n  transient_time=" << transient_time()
       << ",\n  transient_steps=" << transient_steps()
       << ",\n  lock_to_grid_steps=" << lock_to_grid_steps()
       << ",\n  lock_to_grid_time=" << lock_to_grid_time()
       << ",\n  maximum_grid_fineness=" << maximum_grid_fineness()
       << ",\n  maximum_grid_extent=" << maximum_grid_extent()
       << ",\n  bounding_domain=" << (bounding_domain_ptr() ? "available" : "none")
       << ",\n  grid=" << grid()
       << ",\n  paver=" << paver()
       << ",\n  outer_overspill_policy=" << outer_overspill_policy()
       << "\n)\n";
    return os;
}

Void HybridReachabilityAnalyserConfiguration::set_bounding_domain(const HybridExactBoxes& bounding_domain) {
    set_bounding_domain_ptr(std::make_shared<HybridExactBoxes>(bounding_domain));
}

Void HybridReachabilityAnalyserConfiguration::set_bounding_domain_ptr(const std::shared_ptr<HybridExactBoxes> value) {
    //   ARIADNE_ASSERT_MSG(possibly(value_ptr->space() == _analyser.system().state_space()),
    //           "The bounding domain to set has a different hybrid space than the system.");

    _bounding_domain_ptr = value;
}

Void HybridReachabilityAnalyserConfiguration::set_grid(const std::shared_ptr<HybridGrid> value_ptr) {
    ARIADNE_ASSERT_MSG(possibly(value_ptr->space() == _analyser.system().state_space()),
                       "The grid to set has a different hybrid space than the system.");

    _grid_ptr = value_ptr;
}

} // namespace Ariadne

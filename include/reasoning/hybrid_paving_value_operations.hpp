/***************************************************************************
 *            reasoning/hybrid_paving_value_operations.hpp
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

/** @file reasoning/hybrid_paving_value_operations.hpp
 *  @brief System-dependent operations on generic hybrid paving values.
 */

#ifndef ARIADNE_REASONING_HYBRID_PAVING_VALUE_OPERATIONS_HPP
#define ARIADNE_REASONING_HYBRID_PAVING_VALUE_OPERATIONS_HPP

#include "reasoning/hybrid_paving_value_adapters.hpp"

namespace Ariadne {

class HybridAutomatonInterface;

/** @brief Extend the materialized hybrid state pavings with auxiliary variables.
 *  @details The generic storage does not own a scaling policy or a paving
 *  backend factory: both are supplied explicitly. Preserve the selected
 *  tree/ROBDD backend and share its manager through the backend capture.
 *  Missing state locations stay unmaterialized in the result. The input grid
 *  at every materialized location must match the supplied scaling policy.
 */
Hybrid<Paving> extend_auxiliary(
    const Hybrid<Paving>& state_pavings,
    Hybrid<Scalings> scalings,
    std::function<Paving(const Grid&)> backend,
    const HybridAutomatonInterface& automaton);

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_PAVING_VALUE_OPERATIONS_HPP

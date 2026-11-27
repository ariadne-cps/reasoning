/***************************************************************************
 *            reasoning/hybrid_grid_compat.hpp
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

/** @file reasoning/hybrid_grid_compat.hpp
 *  @brief Legacy hybrid grid and scaling policies adapted to generic values.
 */

#ifndef ARIADNE_REASONING_HYBRID_GRID_COMPAT_HPP
#define ARIADNE_REASONING_HYBRID_GRID_COMPAT_HPP

#include "hybrid/hybrid_scalings.hpp"
#include "reasoning/hybrid_grid_value.hpp"

namespace Ariadne {

/** @brief Materialize a concrete grid from a legacy location-aware scaling policy.
 *  @details The supplied RealSpace determines coordinate order, also when the
 *  auxiliary coordinates differ from the location's state coordinates.
 *  This adapter uses scaling(location, variable), not the removed legacy
 *  HybridScalings::grid method, and is independent of HybridGrid internals.
 */
Grid grid_from_scalings(const HybridScalings& policy,
                        const DiscreteLocation& location, const RealSpace& space);

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_GRID_COMPAT_HPP

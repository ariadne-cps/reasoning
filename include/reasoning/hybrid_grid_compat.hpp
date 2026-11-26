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

#include "evolution/scalings.hpp"
#include "hybrid/hybrid_grid.hpp"
#include "hybrid/hybrid_scalings.hpp"
#include "hybrid/value.hpp"
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

// Legacy HybridGrid still exposes the concrete paving Grid API.
// This factory preserves the independent scaling policy in Hybrid<Grid>,
// lazily, without materialising all valid locations.
Hybrid<Grid> make_hybrid_grids(HybridSpace space, HybridScalings scalings);

// Materialize only the variables belonging to the requested location.
// A shared HybridScalings policy may depend on both the location and the
// variable; resolving it lazily preserves that behavior for Hybrid<Scalings>.
Hybrid<Scalings> make_hybrid_scalings(HybridSpace space, HybridScalings policy);

/** @brief Adapt a generic location-indexed scaling family back to a legacy policy.
 *  @details The policy resolves scaling(location, variable) through the
 *  generic values, materializing lazily via the family's own factory when
 *  present. The family must carry its HybridSpace.
 */
HybridScalings make_hybrid_scalings_policy(Hybrid<Scalings> scalings);

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_GRID_COMPAT_HPP

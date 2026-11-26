/***************************************************************************
 *            reasoning/hybrid_grid_value.hpp
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

/** @file reasoning/hybrid_grid_value.hpp
 *  @brief Materialize paving grids from generic lazy scaling values.
 */

#ifndef ARIADNE_REASONING_HYBRID_GRID_VALUE_HPP
#define ARIADNE_REASONING_HYBRID_GRID_VALUE_HPP

#include "evolution/scalings.hpp"
#include "hybrid/value.hpp"
#include "paving/grid.hpp"

namespace Ariadne {

/** @brief Materialize the ordered grid lengths for a location-independent scaling. */
Grid grid_from_scalings(const Scalings& scalings, const RealSpace& space);

/** @brief Lazily build one grid per location from generic scaling values.
 *  @details The captured scaling family is copied into the provider. Neither
 *  scaling policies nor grids are eagerly materialized. Location-dependent
 *  coordinate order is supplied by the requested HybridSpace.
 */
Hybrid<Grid> make_hybrid_grids(HybridSpace space, Hybrid<Scalings> scalings);

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_GRID_VALUE_HPP

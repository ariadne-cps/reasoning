/***************************************************************************
 *            reasoning/hybrid_paving_value_adapters.hpp
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

/** @file reasoning/hybrid_paving_value_adapters.hpp
 *  @brief Lazy paving backends stored in generic location-indexed values.
 */

#ifndef ARIADNE_REASONING_HYBRID_PAVING_VALUE_ADAPTERS_HPP
#define ARIADNE_REASONING_HYBRID_PAVING_VALUE_ADAPTERS_HPP

#include <functional>
#include <utility>

#include "hybrid/value.hpp"
#include "paving/paving_interface.hpp"
#include "reasoning/hybrid_grid_value_adapters.hpp"

namespace Ariadne {

/** @brief Construct a lazy family of pavings with a caller-selected backend.
 *  @details Grids and paving backends are materialized on first mutable
 *  access to a valid location. The backend factory may capture a shared
 *  RobddManager; this capture is retained by the lazy provider. The chosen
 *  backend must preserve the input grid.
 *
 *  @pre The grid family describes the same location spaces as space.
 *  @pre backend is callable.
 */
inline Hybrid<Paving> make_hybrid_pavings(
    HybridSpace space, Hybrid<Grid> grids,
    std::function<Paving(const Grid&)> backend) {
    ARIADNE_PRECONDITION(static_cast<bool>(backend));
    return Hybrid<Paving>(std::move(space),
        [grids = std::move(grids), backend = std::move(backend)](
            const DiscreteLocation& location, const RealSpace& real_space) mutable {
            ARIADNE_PRECONDITION(grids.has_location(location));
            ARIADNE_PRECONDITION(grids.space()[location] == real_space);
            const Grid& grid = grids[location];
            Paving result = backend(grid);
            ARIADNE_PRECONDITION(result.grid() == grid);
            return result;
        });
}

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_PAVING_VALUE_ADAPTERS_HPP

/***************************************************************************
 *            reasoning/hybrid_paving_flatten.hpp
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

/** @file reasoning/hybrid_paving_flatten.hpp
 *  @brief Select locations and expose ordinary Euclidean pavings.
 */

#ifndef ARIADNE_REASONING_HYBRID_PAVING_FLATTEN_HPP
#define ARIADNE_REASONING_HYBRID_PAVING_FLATTEN_HPP

#include "hybrid/hybrid_paving.hpp"
#include "hybrid/value.hpp"
#include "labelled/figure.hpp"
#include "paving/paving_interface.hpp"

namespace Ariadne {

// Flatten location-indexed storage into ordinary continuous sets before any
// rendering. No hybrid drawing object, backend conversion or new dependency
// in the Drawing module is needed. An empty location filter selects all
// materialised locations, as with the legacy HybridPaving::draw.
template<class Visitor>
Void for_each_flattened_paving(const HybridPaving& storage,
                              const Set<DiscreteLocation>& locations,
                              const Variables2d& axes, Visitor&& visitor) {
    for (auto it = storage.locations_begin(); it != storage.locations_end(); ++it) {
        if (locations.empty() || locations.contains(it->first)) {
            const RealSpace space = storage.space(it->first);
            const Projection2d projection(
                space.dimension(), space.index(axes.x()), space.index(axes.y()));
            visitor(projection, static_cast<const SubPavingInterface&>(it->second));
        }
    }
}


// Project a single location-associated value into an ordinary continuous
// coordinate system. A Located<T> carries the location but not a RealSpace,
// so the caller supplies the corresponding variable ordering explicitly.
// This helper is independent of the value type and of the rendering backend.
template<class T, class Visitor>
Void with_projected_location(const Located<T>& located, const RealSpace& space,
                             const Variables2d& axes, Visitor&& visitor) {
    const Projection2d projection(
        space.dimension(), space.index(axes.x()), space.index(axes.y()));
    visitor(projection, located.value());
}

// Select one materialised location from a generic Hybrid<T> family.
// No lazy materialisation occurs through this const access; callers must
// populate the location before requesting a drawing.
template<class T, class Visitor>
Void with_projected_location(const Hybrid<T>& family, const DiscreteLocation& location,
                             const RealSpace& space, const Variables2d& axes,
                             Visitor&& visitor) {
    ARIADNE_PRECONDITION(family.has_value(location));
    const Projection2d projection(
        space.dimension(), space.index(axes.x()), space.index(axes.y()));
    visitor(projection, family[location]);
}

// For location-dependent spaces the variable ordering is taken from the
// family's declared HybridSpace, rather than reconstructed from the value.
template<class T, class Visitor>
Void with_projected_location(const Hybrid<T>& family, const DiscreteLocation& location,
                             const Variables2d& axes, Visitor&& visitor) {
    ARIADNE_PRECONDITION(family.has_value(location));
    const RealSpace space = family.space()[location];
    with_projected_location(family, location, space, axes,
                            std::forward<Visitor>(visitor));
}

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_PAVING_FLATTEN_HPP

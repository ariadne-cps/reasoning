/***************************************************************************
 *            src/hybrid_grid.cpp
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

/** @file src/hybrid_grid.cpp
 *  @brief Materialize generic and legacy hybrid grids from scaling policies.
 */

#include <utility>

#include "algebra/vector.hpp"
#include "reasoning/hybrid_grid_compat.hpp"
#include "reasoning/hybrid_grid_value.hpp"

namespace Ariadne {

Grid grid_from_scalings(const Scalings& scalings, const RealSpace& space) {
    Vector<ExactDouble> lengths(space.dimension(), 1.0_x);
    for (SizeType axis = 0; axis < space.dimension(); ++axis) {
        lengths[axis] = scalings.scaling(space[axis]);
    }
    return Grid(lengths);
}

Grid grid_from_scalings(const HybridScalings& policy,
                        const DiscreteLocation& location, const RealSpace& space) {
    Vector<ExactDouble> lengths(space.dimension());
    for (SizeType axis = 0; axis < space.dimension(); ++axis) {
        lengths[axis] = policy.scaling(location, space[axis]);
    }
    return Grid(lengths);
}

Hybrid<Grid> make_hybrid_grids(HybridSpace space, Hybrid<Scalings> scalings) {
    return Hybrid<Grid>(std::move(space),
                        [scalings = std::move(scalings)](const DiscreteLocation& location,
                                                         const RealSpace& real_space) mutable {
                            return grid_from_scalings(scalings[location], real_space);
                        });
}

Hybrid<Grid> make_hybrid_grids(HybridSpace space, HybridScalings scalings) {
    return Hybrid<Grid>(std::move(space),
                        [scalings = std::move(scalings)](const DiscreteLocation& location,
                                                         const RealSpace& real_space) {
                            return grid_from_scalings(scalings, location, real_space);
                        });
}

Hybrid<Scalings> make_hybrid_scalings(HybridSpace space, HybridScalings policy) {
    return Hybrid<Scalings>(std::move(space),
                            [policy = std::move(policy)](const DiscreteLocation& location,
                                                         const RealSpace& real_space) {
                                Map<RealVariable, ExactDouble> entries;
                                for (SizeType index = 0; index < real_space.dimension(); ++index) {
                                    const RealVariable& variable = real_space[index];
                                    entries.insert(variable, policy.scaling(location, variable));
                                }
                                // All variables of this location have explicit scaling values.
                                // The fallback is only used for names outside its RealSpace.
                                return Scalings(1.0_x, entries);
                            });
}

namespace {
// Legacy policy backed by a generic location-indexed scaling family. Reads
// materialize lazily through the family's own factory when present.
class ValueBackedHybridScalings final : public HybridScalingsInterface {
    mutable Hybrid<Scalings> _values;

  public:
    explicit ValueBackedHybridScalings(Hybrid<Scalings> values) : _values(std::move(values)) {}
    ValueBackedHybridScalings* clone() const override { return new ValueBackedHybridScalings(*this); }
    ExactDouble scaling(const DiscreteLocation& location, const RealVariable& variable) const override {
        ARIADNE_PRECONDITION(_values.has_location(location));
        return _values[location].scaling(variable);
    }
    Void _write(OutputStream& os) const override { os << _values; }
};
} // namespace

HybridScalings make_hybrid_scalings_policy(Hybrid<Scalings> scalings) {
    return HybridScalings(ValueBackedHybridScalings(std::move(scalings)));
}

} // namespace Ariadne

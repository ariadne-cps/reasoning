/***************************************************************************
 *            hybrid_augmented_paving_orbit.cpp
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

/** @file src/hybrid_augmented_paving_orbit.cpp
 *  @brief Implement hybrid augmented paving orbits.
 */

#include "reasoning/hybrid_augmented_paving_orbit.hpp"

namespace Ariadne {

struct Orbit<HybridAugmentedPaving>::Data {
    Data(const HybridGrid& grid, const HybridAuxiliaryMapping& aux)
        : initial(grid, aux), reach(grid, aux), intermediate(grid, aux), final(grid, aux) {}
    HybridAugmentedPaving initial;
    HybridAugmentedPaving reach;
    HybridAugmentedPaving intermediate;
    HybridAugmentedPaving final;
};

Orbit<HybridAugmentedPaving>::
    Orbit(const HybridAugmentedPaving& initial_set)
    : _data(new Data(initial_set.grid(), initial_set.auxiliary_data())) {
    this->_data->initial = initial_set;
}

Orbit<HybridAugmentedPaving>::
    Orbit(const HybridAugmentedPaving& initial_set,
          const HybridAugmentedPaving& reach_set,
          const HybridAugmentedPaving& intermediate_set,
          const HybridAugmentedPaving& final_set)
    : _data(new Data(initial_set.grid(), initial_set.auxiliary_data())) {
    this->_data->initial = initial_set;
    this->_data->reach = reach_set;
    this->_data->intermediate = intermediate_set;
    this->_data->final = final_set;
}

const HybridAugmentedPaving&
    Orbit<HybridAugmentedPaving>::
        initial() const {
    return this->_data->initial;
}

const HybridAugmentedPaving&
    Orbit<HybridAugmentedPaving>::
        reach() const {
    return this->_data->reach;
}

const HybridAugmentedPaving&
    Orbit<HybridAugmentedPaving>::
        intermediate() const {
    return this->_data->intermediate;
}

const HybridAugmentedPaving&
    Orbit<HybridAugmentedPaving>::
        final() const {
    return this->_data->final;
}

} // namespace Ariadne

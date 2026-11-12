/***************************************************************************
 *            augmented_paving_orbit.cpp
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

/** @file src/augmented_paving_orbit.cpp
 *  @brief Implement continuous augmented paving orbits.
 */

#include "reasoning/augmented_paving_orbit.hpp"

namespace Ariadne {

Orbit<AugmentedPaving>::
Orbit(const AugmentedPaving& initial_set)
    : _data(new Orbit<AugmentedPaving>::Data(initial_set.grid(),initial_set.auxiliary_mapping()))
{
    this->_data->initial=initial_set;
}


Orbit<AugmentedPaving>::
Orbit(const AugmentedPaving& initial_set,
      const AugmentedPaving& reach_set,
      const AugmentedPaving& intermediate_set,
      const AugmentedPaving& final_set)
    : _data(new Data(initial_set.grid(),initial_set.auxiliary_mapping()))
{
    this->_data->initial=initial_set;
    this->_data->reach=reach_set;
    this->_data->intermediate=intermediate_set;
    this->_data->final=final_set;
}


AugmentedPaving const&
Orbit<AugmentedPaving>::
initial() const
{
    return this->_data->initial;
}

AugmentedPaving const&
Orbit<AugmentedPaving>::
reach() const
{
    return this->_data->reach;
}

AugmentedPaving const&
Orbit<AugmentedPaving>::
intermediate() const
{
    return this->_data->intermediate;
}

AugmentedPaving const&
Orbit<AugmentedPaving>::
final() const
{
    return this->_data->final;
}

} // namespace Ariadne

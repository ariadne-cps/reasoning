/***************************************************************************
 *            reasoning/hybrid_auxiliary_mapping.hpp
 *
 *  Copyright  2011-20  Pieter Collins
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne.
 *
 *  Ariadne is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne.  If not, see <https://www.gnu.org/licenses/>.
 */

/*! \file reasoning/hybrid_auxiliary_mapping.hpp
 *  \brief Lazy location-indexed labelled auxiliary mappings.
 */
#ifndef ARIADNE_REASONING_HYBRID_AUXILIARY_MAPPING_HPP
#define ARIADNE_REASONING_HYBRID_AUXILIARY_MAPPING_HPP
#include <functional>
#include <utility>
#include "reasoning/labelled_mapping.hpp"
#include "hybrid/hybrid_space.hpp"
#include "hybrid/discrete_location.hpp"

namespace Ariadne {

//! Location-indexed auxiliary functions, evaluated lazily and independent of an automaton.
class HybridAuxiliaryMapping {
    // Destroy the hybrid space before the provider, which may own its backing system.
    std::function<LabelledMapping(DiscreteLocation const&)> _mapping;
    HybridSpace _state_auxiliary_space;
  public:
    HybridAuxiliaryMapping(HybridSpace const& space,
                           std::function<LabelledMapping(DiscreteLocation const&)> mapping)
        : _mapping(std::move(mapping)), _state_auxiliary_space(space) { }
    LabelledMapping operator()(DiscreteLocation const& q) const { return _mapping(q); }
    HybridSpace state_auxiliary_space() const { return _state_auxiliary_space; }
};


} // namespace Ariadne
#endif // ARIADNE_REASONING_HYBRID_AUXILIARY_MAPPING_HPP

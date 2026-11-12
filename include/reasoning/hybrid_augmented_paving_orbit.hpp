/***************************************************************************
 *            reasoning/hybrid_augmented_paving_orbit.hpp
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

/** @file reasoning/hybrid_augmented_paving_orbit.hpp
 *  @brief Reachability orbit storage based on hybrid augmented pavings.
 */

#ifndef ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_HPP
#define ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_HPP

#include "hybrid/hybrid_augmented_paving.hpp"
#include "evolution/hybrid_orbit.hpp"

namespace Ariadne {

/** @brief Store initial, reachable, intermediate and final hybrid paving sets. */
template<>
class Orbit<HybridAugmentedPaving>
{
    struct Data;
  public:
    typedef HybridAugmentedPaving EnclosureListType;

    /** @brief Initialize a hybrid orbit from its initial set. */
    Orbit(const HybridAugmentedPaving& initial_set);
    /** @brief Initialize all four phases of a hybrid orbit. */
    Orbit(const HybridAugmentedPaving& initial_set, const HybridAugmentedPaving& reach_set,
          const HybridAugmentedPaving& intermediate_set, const HybridAugmentedPaving& final_set);

    /** @brief Return the initial hybrid paving. */
    HybridAugmentedPaving const& initial() const;
    /** @brief Return the reach hybrid paving. */
    HybridAugmentedPaving const& reach() const;
    /** @brief Return the intermediate hybrid paving. */
    HybridAugmentedPaving const& intermediate() const;
    /** @brief Return the final hybrid paving. */
    HybridAugmentedPaving const& final() const;
  private:
    SharedPointer<Data> _data;
};

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_HPP

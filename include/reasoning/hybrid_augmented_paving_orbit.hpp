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

#include "evolution/hybrid_orbit.hpp"
#include "reasoning/hybrid_augmented_paving_value.hpp"

namespace Ariadne {

/** @brief Store initial, reachable, intermediate and final generic hybrid paving values. */
template<>
class Orbit<HybridAugmentedPavingValue> {
    struct Data;

  public:
    typedef HybridAugmentedPavingValue EnclosureListType;

    /** @brief Initialize a hybrid orbit from its initial generic value. */
    Orbit(const HybridAugmentedPavingValue& initial_set);
    /** @brief Initialize all four phases of a hybrid orbit. */
    Orbit(const HybridAugmentedPavingValue& initial_set, const HybridAugmentedPavingValue& reach_set,
          const HybridAugmentedPavingValue& intermediate_set, const HybridAugmentedPavingValue& final_set);

    /** @brief Return the initial hybrid paving value. */
    const HybridAugmentedPavingValue& initial() const;
    /** @brief Return the reach hybrid paving value. */
    const HybridAugmentedPavingValue& reach() const;
    /** @brief Return the intermediate hybrid paving value. */
    const HybridAugmentedPavingValue& intermediate() const;
    /** @brief Return the final hybrid paving value. */
    const HybridAugmentedPavingValue& final() const;

  private:
    SharedPointer<Data> _data;
};

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_HPP

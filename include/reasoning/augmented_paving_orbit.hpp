/***************************************************************************
 *            reasoning/augmented_paving_orbit.hpp
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

/** @file reasoning/augmented_paving_orbit.hpp
 *  @brief Reachability orbit storage based on augmented pavings.
 */

// Grid-backed storage orbits belong to reasoning, not evolution.
#ifndef ARIADNE_REASONING_AUGMENTED_PAVING_ORBIT_HPP
#define ARIADNE_REASONING_AUGMENTED_PAVING_ORBIT_HPP

#include "evolution/orbit.hpp"
#include "paving/augmented_paving.hpp"

namespace Ariadne {

/** @brief Store initial, reachable, intermediate and final continuous paving sets. */
template<>
class Orbit<AugmentedPaving> {
    struct Data;

  public:
    typedef AugmentedPaving EnclosureListType;

    /** @brief Initialize an orbit with its initial paving. */
    Orbit(const AugmentedPaving& initial);
    /** @brief Initialize all four phases of a reachable orbit. */
    Orbit(const AugmentedPaving& initial, const AugmentedPaving& reach,
          const AugmentedPaving& intermediate, const AugmentedPaving& final);

    /** @brief Return the initial paving. */
    const AugmentedPaving& initial() const;
    /** @brief Return the reachable paving. */
    const AugmentedPaving& reach() const;
    /** @brief Return the intermediate paving. */
    const AugmentedPaving& intermediate() const;
    /** @brief Return the final paving. */
    const AugmentedPaving& final() const;

  private:
    std::shared_ptr<Data> _data;
};

/** @brief Shared data backing an augmented paving orbit. */
struct Orbit<AugmentedPaving>::Data {
    /** @brief Construct empty orbit phases over the supplied grid and mapping. */
    Data(const Grid& grid, const EffectiveVectorMultivariateFunction& auxiliary_mapping)
        : initial(grid, auxiliary_mapping), reach(initial), intermediate(initial), final(initial) {}
    AugmentedPaving initial;
    AugmentedPaving reach;
    AugmentedPaving intermediate;
    AugmentedPaving final;
};

} // namespace Ariadne

#endif // ARIADNE_REASONING_AUGMENTED_PAVING_ORBIT_HPP

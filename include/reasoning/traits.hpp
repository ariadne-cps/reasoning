/***************************************************************************
 *            reasoning/traits.hpp
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

/** @file reasoning/traits.hpp
 *  @brief Type traits connecting system models to reachability representations.
 */

#ifndef ARIADNE_REASONING_TRAITS_HPP
#define ARIADNE_REASONING_TRAITS_HPP

#include "evolution/traits.hpp"
#include "geometry/set_interface.hpp"
#include "hybrid/hybrid_set_interface.hpp"

// AugmentedPaving and set-interface associations are reasoning-level concerns.
// The generic space traits preserve the EuclideanSpace API during staging;
// hybrid specialisation moves those aliases out of the system-level HybridSpace.
namespace Ariadne {

class HybridSpace;
class LabelledAugmentedPaving;
class HybridAugmentedPaving;
class BinaryTreePaving;

/** @brief Associate a dynamical system type with its reachability storage type. */
template<class SYS> struct AnalysisTraits;

/** @brief Use labelled augmented pavings for iterated maps. */
template<> struct AnalysisTraits<IteratedMap> {
    using StorageType = LabelledAugmentedPaving;
};

/** @brief Use labelled augmented pavings for vector fields. */
template<> struct AnalysisTraits<VectorField> {
    using StorageType = LabelledAugmentedPaving;
};

/** @brief Use labelled augmented pavings for differential inclusions. */
template<> struct AnalysisTraits<DifferentialInclusion> {
    using StorageType = LabelledAugmentedPaving;
};

/** @brief Use hybrid augmented pavings for hybrid automata. */
template<> struct AnalysisTraits<HybridAutomatonInterface> {
    using StorageType = HybridAugmentedPaving;
};

/** @brief Select set and approximation interfaces for a state space. */
template<class SPACE> struct SpaceAnalysisTraits {
    using BoundingDomainType = typename SPACE::BoundingDomainType;
    using BoundedSetInterfaceType = typename SPACE::BoundedSetInterfaceType;
    using OvertSetInterfaceType = typename SPACE::OvertSetInterfaceType;
    using OpenSetInterfaceType = typename SPACE::OpenSetInterfaceType;
    using CompactSetInterfaceType = typename SPACE::CompactSetInterfaceType;
    using LocatedSetInterfaceType = typename SPACE::LocatedSetInterfaceType;
    using RegularSetInterfaceType = typename SPACE::RegularSetInterfaceType;
    using RegularLocatedSetInterfaceType = typename SPACE::RegularLocatedSetInterfaceType;
    using SetApproximationType = typename SPACE::SetApproximationType;
};

// EuclideanSpace intentionally describes the geometry only; the
// discretisation used by reachability analysis is a reasoning concern.
// Do not add SetApproximationType back to EuclideanSpace.
/** @brief Specialize analysis interfaces for Euclidean state spaces. */
template<> struct SpaceAnalysisTraits<EuclideanSpace> {
    using BoundingDomainType = EuclideanSpace::BoundingDomainType;
    using BoundedSetInterfaceType = EuclideanSpace::BoundedSetInterfaceType;
    using OvertSetInterfaceType = EuclideanSpace::OvertSetInterfaceType;
    using OpenSetInterfaceType = EuclideanSpace::OpenSetInterfaceType;
    using CompactSetInterfaceType = EuclideanSpace::CompactSetInterfaceType;
    using LocatedSetInterfaceType = EuclideanSpace::LocatedSetInterfaceType;
    using RegularSetInterfaceType = EuclideanSpace::RegularSetInterfaceType;
    using RegularLocatedSetInterfaceType = EuclideanSpace::RegularLocatedSetInterfaceType;
    using SetApproximationType = BinaryTreePaving;
};

/** @brief Specialize analysis interfaces for hybrid state spaces. */
template<> struct SpaceAnalysisTraits<HybridSpace> {
    using BoundingDomainType = HybridExactBoxes;
    using BoundedSetInterfaceType = EffectiveHybridBoundedSetInterface;
    using OvertSetInterfaceType = EffectiveHybridOvertSetInterface;
    using OpenSetInterfaceType = EffectiveHybridOpenSetInterface;
    using CompactSetInterfaceType = EffectiveHybridCompactSetInterface;
    using LocatedSetInterfaceType = EffectiveHybridLocatedSetInterface;
    using RegularSetInterfaceType = EffectiveHybridRegularSetInterface;
    using RegularLocatedSetInterfaceType = EffectiveHybridRegularLocatedSetInterface;
    using SetApproximationType = HybridPaving;
};

} // namespace Ariadne
#endif // ARIADNE_REASONING_TRAITS_HPP

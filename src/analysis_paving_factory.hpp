/***************************************************************************
 *            src/analysis_paving_factory.hpp
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

/** @file src/analysis_paving_factory.hpp
 *  @brief Factories for constructing analysis-specific paving representations.
 */

#ifndef ARIADNE_REASONING_DETAIL_ANALYSIS_PAVING_FACTORY_HPP
#define ARIADNE_REASONING_DETAIL_ANALYSIS_PAVING_FACTORY_HPP

#include "hybrid/hybrid_auxiliary_mapping.hpp"
#include "hybrid/value.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/paving_interface.hpp"
#include "reasoning/hybrid_augmented_paving_value.hpp"
#include "reasoning/labelled_paving_value.hpp"
#include "reasoning/reachability_analyser.hpp"

namespace Ariadne {
class HybridAutomatonInterface;

/** @brief Construct the storage representation selected by a reachability analyser. */
template<class STORAGE>
struct AnalysisPavingFactory {
    /** @brief Construct a storage object using the supplied grid and system. */
    template<class GRID, class SYS>
    static STORAGE make(GRID const& grid, SYS const& system) {
        return STORAGE(grid, system);
    }
};

/** @brief Construct generic hybrid augmented paving values with location-dependent auxiliary mappings. */
template<>
struct AnalysisPavingFactory<HybridAugmentedPaving> {
    /** @brief Construct an empty value on a generic grid family, with a binary-tree backend. */
    static HybridAugmentedPaving make(const Hybrid<Grid>& grids, const HybridAutomatonInterface& system) {
        return make_hybrid_augmented_paving(grids, system);
    }
    /** @brief Lift a generic hybrid state paving to augmented values using the automaton's auxiliary mapping. */
    static HybridAugmentedPaving make(const Hybrid<Paving>& state, const HybridAutomatonInterface& system) {
        return make_hybrid_augmented_paving(state, make_hybrid_auxiliary_mapping(system));
    }
};

/** @brief Construct generic labelled augmented paving values for continuous systems. */
template<>
struct AnalysisPavingFactory<LabelledAugmentedPaving> {
    /** @brief Construct an empty value on a labelled grid, with a binary-tree backend. */
    template<class SYS>
    static LabelledAugmentedPaving make(const LabelledValue<Grid>& grid, const SYS& system) {
        return make_labelled_augmented_paving(Paving(new BinaryTreePaving(grid.value())), system);
    }
    /** @brief Lift a state paving to an augmented value using the system's auxiliary mapping. */
    template<class SYS>
    static LabelledAugmentedPaving make(const Paving& paving, const SYS& system) {
        return make_labelled_augmented_paving(paving, system);
    }
    /** @brief Attach the system's coordinate labels to an existing augmented paving. */
    template<class SYS>
    static LabelledAugmentedPaving make(const AugmentedPaving& paving, const SYS& system) {
        return make_labelled_augmented_paving(paving, system);
    }
};

/** @brief Dispatch analysis paving construction to the matching storage factory. */
template<class STORAGE, class GRID, class SYS>
STORAGE make_analysis_paving(GRID const& grid, SYS const& system) {
    return AnalysisPavingFactory<STORAGE>::make(grid, system);
}
} // namespace Ariadne
#endif // ARIADNE_REASONING_DETAIL_ANALYSIS_PAVING_FACTORY_HPP

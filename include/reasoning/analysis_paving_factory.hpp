/***************************************************************************
 *            reasoning/analysis_paving_factory.hpp
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

/** @file reasoning/analysis_paving_factory.hpp
 *  @brief Factories for constructing analysis-specific paving representations.
 */

#ifndef ARIADNE_REASONING_ANALYSIS_PAVING_FACTORY_HPP
#define ARIADNE_REASONING_ANALYSIS_PAVING_FACTORY_HPP

#include "hybrid/hybrid_augmented_paving.hpp"

namespace Ariadne {
class HybridAutomatonInterface;

/** @brief Adapt an automaton into a lazy, location-indexed auxiliary mapping. */
HybridAuxiliaryMapping make_hybrid_auxiliary_mapping(HybridAutomatonInterface const& system);

/** @brief Construct the storage representation selected by a reachability analyser. */
template<class STORAGE> struct AnalysisPavingFactory {
    /** @brief Construct a storage object using the supplied grid and system. */
    template<class GRID, class SYS>
    static STORAGE make(GRID const& grid, SYS const& system) {
        return STORAGE(grid,system);
    }
};

/** @brief Construct hybrid pavings with location-dependent auxiliary mappings. */
template<> struct AnalysisPavingFactory<HybridAugmentedPaving> {
    /** @brief Construct a hybrid paving using the automaton's auxiliary mapping. */
    template<class GRID>
    static HybridAugmentedPaving make(GRID const& grid, HybridAutomatonInterface const& system) {
        return HybridAugmentedPaving(grid,make_hybrid_auxiliary_mapping(system));
    }
};

/** @brief Dispatch analysis paving construction to the matching storage factory. */
template<class STORAGE, class GRID, class SYS>
STORAGE make_analysis_paving(GRID const& grid, SYS const& system) {
    return AnalysisPavingFactory<STORAGE>::make(grid,system);
}
} // namespace Ariadne
#endif // ARIADNE_REASONING_ANALYSIS_PAVING_FACTORY_HPP

#ifndef ARIADNE_REASONING_ANALYSIS_PAVING_FACTORY_HPP
#define ARIADNE_REASONING_ANALYSIS_PAVING_FACTORY_HPP

#include "hybrid/hybrid_augmented_paving.hpp"

namespace Ariadne {
class HybridAutomatonInterface;

// Adapt an automaton into a lazy, location-indexed auxiliary mapping.
HybridAuxiliaryMapping make_hybrid_auxiliary_mapping(HybridAutomatonInterface const& system);

template<class STORAGE> struct AnalysisPavingFactory {
    template<class GRID, class SYS>
    static STORAGE make(GRID const& grid, SYS const& system) {
        return STORAGE(grid,system);
    }
};

template<> struct AnalysisPavingFactory<HybridAugmentedPaving> {
    template<class GRID>
    static HybridAugmentedPaving make(GRID const& grid, HybridAutomatonInterface const& system) {
        return HybridAugmentedPaving(grid,make_hybrid_auxiliary_mapping(system));
    }
};

template<class STORAGE, class GRID, class SYS>
STORAGE make_analysis_paving(GRID const& grid, SYS const& system) {
    return AnalysisPavingFactory<STORAGE>::make(grid,system);
}
} // namespace Ariadne
#endif // ARIADNE_REASONING_ANALYSIS_PAVING_FACTORY_HPP

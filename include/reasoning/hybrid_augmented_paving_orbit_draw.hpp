// Drawing of the reasoning-owned hybrid augmented paving orbit.
// The hybrid paving itself is rendered by hybrid/hybrid_augmented_paving_draw.hpp.
#ifndef ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_DRAW_HPP
#define ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_DRAW_HPP

#include "reasoning/hybrid_augmented_paving_orbit.hpp"
#include "hybrid/hybrid_augmented_paving_draw.hpp"

namespace Ariadne {
inline Void draw(CanvasInterface& canvas, Set<DiscreteLocation> const& locations,
                 Variables2d const& axes, Orbit<HybridAugmentedPaving> const& orbit) {
    Ariadne::draw(canvas,locations,axes,orbit.reach());
}
} // namespace Ariadne
#endif // ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_DRAW_HPP

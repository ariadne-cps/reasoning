// Temporary adapter; final home will be the drawing repository.
#ifndef ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_DRAW_HPP
#define ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_DRAW_HPP

#include "reasoning/hybrid_augmented_paving.hpp"
#include "reasoning/hybrid_augmented_paving_orbit.hpp"
#include "hybrid/hybrid_drawable_interface.hpp"

namespace Ariadne {

inline Void draw(CanvasInterface& canvas, Set<DiscreteLocation> const& locations,
                 Variables2d const& axes, HybridAugmentedPaving const& paving) {
    paving.state_auxiliary_paving().draw(canvas,locations,axes);
}

class HybridAugmentedPavingDrawable final : public HybridDrawableInterface {
  public:
    explicit HybridAugmentedPavingDrawable(HybridAugmentedPaving const& paving) : _paving(paving) { }
    Void draw(CanvasInterface& canvas, Set<DiscreteLocation> const& locations,
              Variables2d const& axes) const override {
        Ariadne::draw(canvas,locations,axes,_paving);
    }
  private:
    HybridAugmentedPaving _paving;
};

inline Void draw(CanvasInterface& canvas, Set<DiscreteLocation> const& locations,
                 Variables2d const& axes, Orbit<HybridAugmentedPaving> const& orbit) {
    Ariadne::draw(canvas,locations,axes,orbit.reach());
}

} // namespace Ariadne
#endif // ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_DRAW_HPP

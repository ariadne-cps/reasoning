// Temporary adapter; final home will be the drawing repository.
#ifndef ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_DRAW_HPP
#define ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_DRAW_HPP

#include "reasoning/labelled_augmented_paving.hpp"
#include "reasoning/augmented_paving_draw.hpp"
#include "labelled/figure.hpp"

namespace Ariadne {

inline Void draw(CanvasInterface& canvas, Variables2d const& axes, LabelledAugmentedPaving const& paving) {
    Projection2d proj=Ariadne::projection(paving.state_auxiliary_space(),axes);
    Ariadne::draw(canvas,proj,static_cast<AugmentedPaving const&>(paving));
}

class LabelledAugmentedPavingDrawable final : public LabelledDrawable2dInterface {
  public:
    explicit LabelledAugmentedPavingDrawable(LabelledAugmentedPaving const& paving) : _paving(paving) { }
    LabelledAugmentedPavingDrawable* labelled_clone() const override {
        return new LabelledAugmentedPavingDrawable(*this);
    }
    Void draw(CanvasInterface& canvas, Variables2d const& axes) const override {
        Ariadne::draw(canvas,axes,_paving);
    }
  private:
    LabelledAugmentedPaving _paving;
};

// Projection-based figures use the same auxiliary graph as AugmentedPaving.
// A dedicated overload avoids reliance on implicit conversions or on
// mathematical types inheriting rendering interfaces.
inline Void draw_to_figure(Figure& figure, LabelledAugmentedPaving const& paving) {
    draw_to_figure(figure,static_cast<AugmentedPaving const&>(paving));
}
inline Void draw(Figure& figure, LabelledAugmentedPaving const& paving) {
    draw_to_figure(figure,paving);
}
inline Figure& operator<<(Figure& figure, LabelledAugmentedPaving const& paving) {
    draw_to_figure(figure,paving); return figure;
}

inline Void draw_to_figure(LabelledFigure& figure, LabelledAugmentedPaving const& paving) {
    figure.draw(LabelledAugmentedPavingDrawable(paving));
}
inline Void draw(LabelledFigure& figure, LabelledAugmentedPaving const& paving) {
    draw_to_figure(figure,paving);
}
inline LabelledFigure& operator<<(LabelledFigure& figure, LabelledAugmentedPaving const& paving) {
    draw_to_figure(figure,paving); return figure;
}

} // namespace Ariadne
#endif // ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_DRAW_HPP

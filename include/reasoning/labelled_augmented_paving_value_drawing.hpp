/***************************************************************************
 *            reasoning/labelled_augmented_paving_value_drawing.hpp
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

/** @file reasoning/labelled_augmented_paving_value_drawing.hpp
 *  @brief Draw generic labelled augmented paving values without the legacy subclass.
 */
#ifndef ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_VALUE_DRAWING_HPP
#define ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_VALUE_DRAWING_HPP

#include "labelled/figure.hpp"
#include "reasoning/labelled_augmented_paving_adapters.hpp"
#include "reasoning/paving_drawing.hpp"

namespace Ariadne::PavingDrawing {

// Generic value counterpart: the drawing logic remains in Reasoning because
// its AugmentedPaving representation must not enter the Drawing module.
// Figure stores an owned drawable snapshot, just as with the legacy class.
inline Void draw(CanvasInterface& canvas, const Variables2d& axes,
                 const LabelledAugmentedPavingValue& paving) {
    const Projection2d proj = Ariadne::projection(state_auxiliary_space(paving), axes);
    PavingDrawing::draw(canvas, proj, paving.value());
}

class LabelledAugmentedPavingValueDrawable final : public LabelledDrawable2dInterface {
  public:
    explicit LabelledAugmentedPavingValueDrawable(const LabelledAugmentedPavingValue& paving)
        : _paving(paving) {}

    LabelledAugmentedPavingValueDrawable* labelled_clone() const override {
        return new LabelledAugmentedPavingValueDrawable(*this);
    }

    Void draw(CanvasInterface& canvas, const Variables2d& axes) const override {
        PavingDrawing::draw(canvas, axes, _paving);
    }

  private:
    LabelledAugmentedPavingValue _paving;
};

inline Void draw(Figure& figure, const LabelledAugmentedPavingValue& paving) {
    PavingDrawing::draw(figure, paving.value());
}

inline Void draw(LabelledFigure& figure, const LabelledAugmentedPavingValue& paving) {
    figure.draw(LabelledAugmentedPavingValueDrawable(paving));
}

} // namespace Ariadne::PavingDrawing

#endif // ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_VALUE_DRAWING_HPP

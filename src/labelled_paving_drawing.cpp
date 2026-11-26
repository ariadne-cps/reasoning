/***************************************************************************
 *            src/labelled_paving_drawing.cpp
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

/** @file src/labelled_paving_drawing.cpp
 *  @brief Implement the generic and legacy labelled paving drawing adapters.
 */

#include "reasoning/labelled_paving_drawing.hpp"

#include "reasoning/labelled_augmented_paving_value_drawing.hpp"

namespace Ariadne::PavingDrawing {

// Generic value adapters.
Void draw(CanvasInterface& canvas, const Variables2d& axes,
          const LabelledAugmentedPavingValue& paving) {
    const Projection2d proj = Ariadne::projection(state_auxiliary_space(paving), axes);
    PavingDrawing::draw(canvas, proj, paving.value());
}

Void draw(Figure& figure, const LabelledAugmentedPavingValue& paving) {
    PavingDrawing::draw(figure, paving.value());
}

Void draw(LabelledFigure& figure, const LabelledAugmentedPavingValue& paving) {
    figure.draw(LabelledAugmentedPavingValueDrawable(paving));
}

LabelledAugmentedPavingValueDrawable::LabelledAugmentedPavingValueDrawable(
    const LabelledAugmentedPavingValue& paving)
    : _paving(paving) {}

LabelledAugmentedPavingValueDrawable*
    LabelledAugmentedPavingValueDrawable::labelled_clone() const {
    return new LabelledAugmentedPavingValueDrawable(*this);
}

Void LabelledAugmentedPavingValueDrawable::draw(
    CanvasInterface& canvas, const Variables2d& axes) const {
    PavingDrawing::draw(canvas, axes, _paving);
}

// Legacy adapters retained for existing consumers.
Void draw(CanvasInterface& canvas, const Variables2d& axes, const LabelledAugmentedPaving& paving) {
    /** @brief Project coordinates into the requested variable space. */
    Projection2d proj = Ariadne::projection(paving.state_auxiliary_space(), axes);
    PavingDrawing::draw(canvas, proj, static_cast<const AugmentedPaving&>(paving));
}

Void draw_to_figure(Figure& figure, const LabelledAugmentedPaving& paving) {
    PavingDrawing::draw(figure, static_cast<const AugmentedPaving&>(paving));
}

Void draw(Figure& figure, const LabelledAugmentedPaving& paving) {
    /** @brief Perform the labelled geometric operation. */
    PavingDrawing::draw_to_figure(figure, paving);
}

Figure& operator<<(Figure& figure, const LabelledAugmentedPaving& paving) {
    /** @brief Perform the labelled geometric operation. */
    PavingDrawing::draw_to_figure(figure, paving);
    return figure;
}

Void draw_to_figure(LabelledFigure& figure, const LabelledAugmentedPaving& paving) {
    figure.draw(LabelledAugmentedPavingDrawable(paving));
}

Void draw(LabelledFigure& figure, const LabelledAugmentedPaving& paving) {
    /** @brief Perform the labelled geometric operation. */
    PavingDrawing::draw_to_figure(figure, paving);
}

LabelledFigure& operator<<(LabelledFigure& figure, const LabelledAugmentedPaving& paving) {
    /** @brief Perform the labelled geometric operation. */
    PavingDrawing::draw_to_figure(figure, paving);
    return figure;
}

LabelledAugmentedPavingDrawable::LabelledAugmentedPavingDrawable(
    const LabelledAugmentedPaving& paving)
    : _paving(paving) {}

LabelledAugmentedPavingDrawable*
    LabelledAugmentedPavingDrawable::labelled_clone() const {
    return new LabelledAugmentedPavingDrawable(*this);
}

Void LabelledAugmentedPavingDrawable::draw(
    CanvasInterface& canvas, const Variables2d& axes) const {
    PavingDrawing::draw(canvas, axes, _paving);
}

} // namespace Ariadne::PavingDrawing

/***************************************************************************
 *            src/paving_drawing.cpp
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

/** @file src/paving_drawing.cpp
 *  @brief Implement backend-independent paving rendering and drawable ownership.
 */

#include "reasoning/paving_drawing.hpp"

namespace Ariadne::PavingDrawing {

Void draw_coloured_pavings(Figure&) {}

Void draw(CanvasInterface& canvas, const Projection2d& projection,
          const SubPavingInterface& paving) {
    for (auto cell = paving.begin(), end = paving.end(); cell != end; ++cell) {
        Ariadne::draw(canvas, projection, cell->box());
    }
}

Void draw(CanvasInterface& canvas, const Projection2d& projection,
          const GridCell& cell) {
    Ariadne::draw(canvas, projection, cell.box());
}

Void draw(CanvasInterface& canvas, const Projection2d& projection,
          const AugmentedPaving& paving) {
    const BinaryTreePaving graph = paving.state_auxiliary_paving();
    PavingDrawing::draw(canvas, projection, static_cast<const SubPavingInterface&>(graph));
}

Void draw(Figure& figure, const SubPavingInterface& paving) {
    figure.draw(SubPavingDrawable(paving));
}

Void draw(Figure& figure, const Paving& paving) {
    PavingDrawing::draw(figure, static_cast<const PavingInterface&>(paving));
}

Void draw(Figure& figure, const SubPaving& paving) {
    PavingDrawing::draw(figure, static_cast<const SubPavingInterface&>(paving));
}

Void draw(Figure& figure, const GridCell& cell) {
    figure.draw(GridCellDrawable(cell));
}

Void draw(Figure& figure, const AugmentedPaving& paving) {
    figure.draw(AugmentedPavingDrawable(paving));
}

SubPavingDrawable::SubPavingDrawable(const SubPavingInterface& paving)
    : _paving(paving.clone()) {}

SubPavingDrawable::SubPavingDrawable(const SubPavingDrawable& other)
    : _paving(other._paving->clone()) {}

SubPavingDrawable* SubPavingDrawable::clone() const {
    return new SubPavingDrawable(*this);
}

DimensionType SubPavingDrawable::dimension() const {
    return _paving->dimension();
}

Void SubPavingDrawable::draw(CanvasInterface& canvas, const Projection2d& projection) const {
    PavingDrawing::draw(canvas, projection, *_paving);
}

GridCellDrawable::GridCellDrawable(const GridCell& cell) : _cell(cell) {}

GridCellDrawable* GridCellDrawable::clone() const {
    return new GridCellDrawable(*this);
}

DimensionType GridCellDrawable::dimension() const {
    return _cell.dimension();
}

Void GridCellDrawable::draw(CanvasInterface& canvas, const Projection2d& projection) const {
    PavingDrawing::draw(canvas, projection, _cell);
}

AugmentedPavingDrawable::AugmentedPavingDrawable(const AugmentedPaving& paving)
    : _paving(paving) {}

AugmentedPavingDrawable* AugmentedPavingDrawable::clone() const {
    return new AugmentedPavingDrawable(*this);
}

DimensionType AugmentedPavingDrawable::dimension() const {
    return _paving.dimension() + _paving.auxiliary_mapping().result_size();
}

Void AugmentedPavingDrawable::draw(CanvasInterface& canvas, const Projection2d& projection) const {
    PavingDrawing::draw(canvas, projection, _paving);
}

} // namespace Ariadne::PavingDrawing

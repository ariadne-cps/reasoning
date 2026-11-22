/***************************************************************************
 *            reasoning/paving_drawing.hpp
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

/** @file reasoning/paving_drawing.hpp
 *  @brief Drawing adapters for paving geometry, outside the Drawing module.
 */

#ifndef ARIADNE_REASONING_PAVING_DRAWING_HPP
#define ARIADNE_REASONING_PAVING_DRAWING_HPP

#include <memory>

#include "drawing/box_draw.hpp"
#include "paving/augmented_paving.hpp"
#include "paving/binary_tree_paving.hpp"
#include "paving/paving_interface.hpp"
#include "rendering/figure.hpp"

namespace Ariadne::PavingDrawing {

// Render the mathematical union of cells, independently of its storage
// representation (binary tree, ROBDD or a borrowed subpaving view).
inline Void draw(CanvasInterface& canvas, const Projection2d& projection,
                 const SubPavingInterface& paving) {
    for (auto cell = paving.begin(), end = paving.end(); cell != end; ++cell) {
        Ariadne::draw(canvas, projection, cell->box());
    }
}

inline Void draw(CanvasInterface& canvas, const Projection2d& projection,
                 const GridCell& cell) {
    Ariadne::draw(canvas, projection, cell.box());
}

// The auxiliary graph is a derived paving, not the authoritative state store.
inline Void draw(CanvasInterface& canvas, const Projection2d& projection,
                 const AugmentedPaving& paving) {
    const BinaryTreePaving graph = paving.state_auxiliary_paving();
    draw(canvas, projection, static_cast<const SubPavingInterface&>(graph));
}

// Figure keeps independent copies, including borrowed subpaving views.
class SubPavingDrawable final : public Drawable2dInterface {
    std::unique_ptr<SubPavingInterface> _paving;

  public:
    explicit SubPavingDrawable(const SubPavingInterface& paving)
        : _paving(paving.clone()) {}
    SubPavingDrawable(const SubPavingDrawable& other)
        : _paving(other._paving->clone()) {}
    SubPavingDrawable* clone() const override { return new SubPavingDrawable(*this); }
    DimensionType dimension() const override { return _paving->dimension(); }
    Void draw(CanvasInterface& canvas, const Projection2d& projection) const override {
        PavingDrawing::draw(canvas, projection, *_paving);
    }
};

class GridCellDrawable final : public Drawable2dInterface {
    GridCell _cell;

  public:
    explicit GridCellDrawable(const GridCell& cell) : _cell(cell) {}
    GridCellDrawable* clone() const override { return new GridCellDrawable(*this); }
    DimensionType dimension() const override { return _cell.dimension(); }
    Void draw(CanvasInterface& canvas, const Projection2d& projection) const override {
        PavingDrawing::draw(canvas, projection, _cell);
    }
};

class AugmentedPavingDrawable final : public Drawable2dInterface {
    AugmentedPaving _paving;

  public:
    explicit AugmentedPavingDrawable(const AugmentedPaving& paving) : _paving(paving) {}
    AugmentedPavingDrawable* clone() const override {
        return new AugmentedPavingDrawable(*this);
    }
    DimensionType dimension() const override {
        return _paving.dimension() + _paving.auxiliary_mapping().result_size();
    }
    Void draw(CanvasInterface& canvas, const Projection2d& projection) const override {
        PavingDrawing::draw(canvas, projection, _paving);
    }
};

inline Void draw(Figure& figure, const SubPavingInterface& paving) {
    figure.draw(SubPavingDrawable(paving));
}

inline Void draw(Figure& figure, const Paving& paving) {
    draw(figure, static_cast<const PavingInterface&>(paving));
}

inline Void draw(Figure& figure, const SubPaving& paving) {
    draw(figure, static_cast<const SubPavingInterface&>(paving));
}

inline Void draw(Figure& figure, const GridCell& cell) {
    figure.draw(GridCellDrawable(cell));
}

inline Void draw(Figure& figure, const AugmentedPaving& paving) {
    figure.draw(AugmentedPavingDrawable(paving));
}

} // namespace Ariadne::PavingDrawing

#endif // ARIADNE_REASONING_PAVING_DRAWING_HPP

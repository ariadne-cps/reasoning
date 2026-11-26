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
 *  @brief Drawing and plotting adapters for paving geometry in Reasoning.
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
Void draw(CanvasInterface& canvas, const Projection2d& projection,
                 const SubPavingInterface& paving);

Void draw(CanvasInterface& canvas, const Projection2d& projection,
                 const GridCell& cell);

// The auxiliary graph is a derived paving, not the authoritative state store.
Void draw(CanvasInterface& canvas, const Projection2d& projection,
                 const AugmentedPaving& paving);

// Figure keeps independent copies, including borrowed subpaving views.
class SubPavingDrawable final : public Drawable2dInterface {
    std::unique_ptr<SubPavingInterface> _paving;

  public:
    explicit SubPavingDrawable(const SubPavingInterface& paving);
    SubPavingDrawable(const SubPavingDrawable& other);
    SubPavingDrawable* clone() const override;
    DimensionType dimension() const override;
    Void draw(CanvasInterface& canvas, const Projection2d& projection) const override;
};

class GridCellDrawable final : public Drawable2dInterface {
    GridCell _cell;

  public:
    explicit GridCellDrawable(const GridCell& cell);
    GridCellDrawable* clone() const override;
    DimensionType dimension() const override;
    Void draw(CanvasInterface& canvas, const Projection2d& projection) const override;
};

class AugmentedPavingDrawable final : public Drawable2dInterface {
    AugmentedPaving _paving;

  public:
    explicit AugmentedPavingDrawable(const AugmentedPaving& paving);
    AugmentedPavingDrawable* clone() const override;
    DimensionType dimension() const override;
    Void draw(CanvasInterface& canvas, const Projection2d& projection) const override;
};

Void draw(Figure& figure, const SubPavingInterface& paving);

Void draw(Figure& figure, const Paving& paving);

Void draw(Figure& figure, const SubPaving& paving);

Void draw(Figure& figure, const GridCell& cell);

Void draw(Figure& figure, const AugmentedPaving& paving);

// Preserve the legacy colour/order sequence while accepting any
// AugmentedPaving, including its labelled legacy subclass. No labelled
// representation is required by this plotting interface.
Void draw_coloured_pavings(Figure&);

template<class... Remaining>
Void draw_coloured_pavings(Figure& figure, const Colour& colour,
                          const AugmentedPaving& paving,
                          const Remaining&... remaining) {
    figure.set_fill_colour(colour);
    PavingDrawing::draw(figure, paving);
    draw_coloured_pavings(figure, remaining...);
}

template<class BoundingBox, class... Pavings>
Void plot(const char* filename, const Projection2d& projection,
          const BoundingBox& bounds, const Pavings&... pavings) {
    Figure figure(bounds, projection);
    draw_coloured_pavings(figure, pavings...);
    figure.write(filename);
}

} // namespace Ariadne::PavingDrawing

#endif // ARIADNE_REASONING_PAVING_DRAWING_HPP

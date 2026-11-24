/***************************************************************************
 *            reasoning/augmented_paving_plot.hpp
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

/** @file reasoning/augmented_paving_plot.hpp
 *  @brief Plot coloured augmented pavings without labelled-paving specialisations.
 */
#ifndef ARIADNE_REASONING_AUGMENTED_PAVING_PLOT_HPP
#define ARIADNE_REASONING_AUGMENTED_PAVING_PLOT_HPP

#include "reasoning/paving_drawing.hpp"

namespace Ariadne::PavingDrawing {

// Preserve the legacy colour/order sequence while accepting any
// AugmentedPaving, including its labelled legacy subclass. No labelled
// representation is required by this plotting interface.
inline Void draw_coloured_pavings(Figure&) {}

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

#endif // ARIADNE_REASONING_AUGMENTED_PAVING_PLOT_HPP

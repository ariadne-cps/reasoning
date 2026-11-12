/***************************************************************************
 *            ariadne-reasoning.hpp
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

/** @file ariadne-reasoning.hpp
 *  @brief Umbrella header for reachability analysis and symbolic reasoning.
 */

#ifndef ARIADNE_ARIADNE_REASONING_HPP
#define ARIADNE_ARIADNE_REASONING_HPP
#include "ariadne-evolution.hpp"
#include "ariadne-sat.hpp"
#include "ariadne-smt.hpp"
#include "reasoning/traits.hpp"
#include "reasoning/neural_symbolic.hpp"
#include "paving/augmented_paving.hpp"
#include "labelled/labelled_grid.hpp"
#include "labelled/labelled_mapping.hpp"
#include "labelled/labelled_augmented_paving.hpp"
#include "hybrid/hybrid_auxiliary_mapping.hpp"
#include "reasoning/augmented_paving_orbit.hpp"
#include "hybrid/hybrid_augmented_paving.hpp"
#include "reasoning/hybrid_augmented_paving_orbit.hpp"
#include "evolution/enclosure_augmented_paving.hpp"
#include "reasoning/reachability_analyser_interface.hpp"
#include "reasoning/reachability_analyser.hpp"
#include "reasoning/hybrid_reachability_analyser_interface.hpp"
#include "reasoning/hybrid_reachability_analyser.hpp"
#endif // ARIADNE_ARIADNE_REASONING_HPP

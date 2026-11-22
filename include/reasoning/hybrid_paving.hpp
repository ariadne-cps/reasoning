/***************************************************************************
 *            reasoning/hybrid_paving.hpp
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

/** @file reasoning/hybrid_paving.hpp
 *  @brief Auxiliary-variable extensions for hybrid pavings.
 */

#ifndef ARIADNE_REASONING_HYBRID_PAVING_HPP
#define ARIADNE_REASONING_HYBRID_PAVING_HPP

#include "hybrid/hybrid_paving.hpp"

namespace Ariadne {

/** @brief HybridAutomatonInterface for auxiliary-variable extensions for hybrid pavings. */
class HybridAutomatonInterface;

//! Extend a hybrid state paving with the auxiliary variables of a system.
HybridPaving extend_auxiliary(const HybridPaving& hgtp,
                              const HybridAutomatonInterface& ha);

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_PAVING_HPP

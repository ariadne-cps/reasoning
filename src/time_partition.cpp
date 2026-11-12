/***************************************************************************
 *            time_partition.cpp
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

/** @file src/time_partition.cpp
 *  @brief Compute conservative whole-step partitions of continuous time.
 */

#include "reasoning/time_partition.hpp"

#include "numeric/accuracy.hpp"
#include "numeric/dyadic.hpp"
#include "numeric/validated_real.hpp"

namespace Ariadne::ReasoningDetail {

Natural whole_time_steps(const Real& time, const Real& grid_time) {
    auto quotient = (time / grid_time).compute(Accuracy(16_bits)).get();
    // A Real's exact floor need not be decidable at an integer boundary.
    // The lower bound safely undercounts there; the remainder step then
    // covers the outstanding time without advancing past the target.
    return cast_positive(max(Integer(0), floor(quotient.lower())));
}

} // namespace Ariadne::ReasoningDetail

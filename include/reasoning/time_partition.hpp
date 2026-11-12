/***************************************************************************
 *            reasoning/time_partition.hpp
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

/** @file reasoning/time_partition.hpp
 *  @brief Partitioning continuous time into whole grid steps.
 */

#ifndef ARIADNE_REASONING_TIME_PARTITION_HPP
#define ARIADNE_REASONING_TIME_PARTITION_HPP
#include "numeric/real.hpp"
#include "numeric/integer.hpp"

namespace Ariadne::ReasoningDetail {
/** @brief Choose whole grid steps without exceeding the continuous time horizon. */
Natural whole_time_steps(Real const& time,Real const& grid_time);
}
#endif

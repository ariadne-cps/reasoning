/***************************************************************************
 *            reasoning/parallel_reachability.hpp
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

/** @file reasoning/parallel_reachability.hpp
 *  @brief Parallel reachability lane execution and rounding-mode validation.
 */

#ifndef ARIADNE_REASONING_PARALLEL_REACHABILITY_HPP
#define ARIADNE_REASONING_PARALLEL_REACHABILITY_HPP

#include <cstddef>
#include <functional>

namespace Ariadne {
/** @brief Thread manager used for parallel reachability execution. */
class ThreadManager;
namespace ReasoningDetail {
/** @brief Check the floating-point rounding mode required by reachability workers. */
int require_caller_rounding_mode(int mode);
/** @brief Execute independent reachability lanes using the supplied thread manager. */
void run_reachability_lanes(std::size_t count,
                            const std::function<void(std::size_t)>& process, ThreadManager& threading);
} // namespace ReasoningDetail
} // namespace Ariadne
#endif

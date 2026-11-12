/***************************************************************************
 *            parallel_reachability.cpp
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

/** @file src/parallel_reachability.cpp
 *  @brief Run independent reachability lanes while preserving rounding modes.
 */

#include "reasoning/parallel_reachability.hpp"

#include <exception>
#include <stdexcept>

#include "threading/task_group.hpp"
#include "threading/thread_manager.hpp"

namespace Ariadne::ReasoningDetail {

int require_caller_rounding_mode(int mode) {
    if (mode < 0)
        throw std::runtime_error("Cannot read caller rounding mode");
    return mode;
}

void run_reachability_lanes(std::size_t count,
                            const std::function<void(std::size_t)>& process, ThreadManager& threading) {
    if (count == 0u)
        return;
    TaskGroup<ThreadManager> tasks(threading);
    for (std::size_t lane = 1u; lane < count; ++lane)
        tasks.fork([&, lane] { process(lane); });
    std::exception_ptr failure;
    try {
        process(0u);
    } catch (...) {
        failure = std::current_exception();
    }
    try {
        tasks.join();
    } catch (...) {
        if (not failure)
            failure = std::current_exception();
    }
    if (failure)
        std::rethrow_exception(failure);
}

} // namespace Ariadne::ReasoningDetail

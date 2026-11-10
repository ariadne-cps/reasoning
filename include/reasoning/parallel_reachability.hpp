#ifndef ARIADNE_REASONING_PARALLEL_REACHABILITY_HPP
#define ARIADNE_REASONING_PARALLEL_REACHABILITY_HPP

#include <cstddef>
#include <functional>

namespace Ariadne {
class ThreadManager;
namespace ReasoningDetail {
int require_caller_rounding_mode(int mode);
void run_reachability_lanes(std::size_t count,
    std::function<void(std::size_t)> const& process,ThreadManager& threading);
}
}
#endif

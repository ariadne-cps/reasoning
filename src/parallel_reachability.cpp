#include "reasoning/parallel_reachability.hpp"
#include "threading/task_group.hpp"
#include "threading/thread_manager.hpp"
#include <exception>
#include <stdexcept>

namespace Ariadne::ReasoningDetail {

int require_caller_rounding_mode(int mode) {
    if(mode<0) throw std::runtime_error("Cannot read caller rounding mode");
    return mode;
}

void run_reachability_lanes(std::size_t count,
        std::function<void(std::size_t)> const& process,ThreadManager& threading) {
    if(count==0u) return;
    TaskGroup<ThreadManager> tasks(threading);
    for(std::size_t lane=1u;lane<count;++lane)
        tasks.fork([&,lane] { process(lane); });
    std::exception_ptr failure;
    try { process(0u); }
    catch(...) { failure=std::current_exception(); }
    try { tasks.join(); }
    catch(...) { if(not failure) failure=std::current_exception(); }
    if(failure) std::rethrow_exception(failure);
}

}

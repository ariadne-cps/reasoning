#ifndef ARIADNE_REASONING_TIME_PARTITION_HPP
#define ARIADNE_REASONING_TIME_PARTITION_HPP
#include "numeric/real.hpp"
#include "numeric/integer.hpp"

namespace Ariadne::ReasoningDetail {
// Choose whole grid steps without exceeding the requested continuous time.
Natural whole_time_steps(Real const& time,Real const& grid_time);
}
#endif

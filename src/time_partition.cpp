#include "reasoning/time_partition.hpp"
#include "numeric/dyadic.hpp"
#include "numeric/accuracy.hpp"
#include "numeric/validated_real.hpp"

namespace Ariadne::ReasoningDetail {

Natural whole_time_steps(Real const& time,Real const& grid_time) {
    auto quotient=(time/grid_time).compute(Accuracy(16_bits)).get();
    // A Real's exact floor need not be decidable at an integer boundary.
    // The lower bound safely undercounts there; the remainder step then
    // covers the outstanding time without advancing past the target.
    return cast_positive(max(Integer(0),floor(quotient.lower())));
}

}

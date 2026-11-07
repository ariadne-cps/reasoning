// Verify that this header is self-contained and does not rely on include order.
#include "reasoning/hybrid_auxiliary_mapping.hpp"

static_assert(sizeof(Ariadne::HybridAuxiliaryMapping) > 0);

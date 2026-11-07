// Verify that this header is self-contained and does not rely on include order.
#include "reasoning/hybrid_augmented_paving.hpp"

static_assert(sizeof(Ariadne::HybridAugmentedPaving) > 0);

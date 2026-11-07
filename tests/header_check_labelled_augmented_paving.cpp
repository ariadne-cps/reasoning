// Verify that this header is self-contained and does not rely on include order.
#include "reasoning/labelled_augmented_paving.hpp"

static_assert(sizeof(Ariadne::LabelledAugmentedPaving) > 0);

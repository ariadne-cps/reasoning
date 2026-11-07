// Verify that this header is self-contained and does not rely on include order.
#include "reasoning/labelled_mapping.hpp"

static_assert(sizeof(Ariadne::LabelledMapping) > 0);

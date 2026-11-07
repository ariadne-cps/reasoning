// Verify that this header is self-contained and does not rely on include order.
#include "reasoning/labelled_grid.hpp"

static_assert(sizeof(Ariadne::LabelledGrid) > 0);

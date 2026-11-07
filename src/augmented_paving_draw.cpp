// The rendering adapter is deliberately kept outside the mathematical
// AugmentedPaving header; the paving itself needs no drawing declarations.
#include "reasoning/augmented_paving.hpp"
#include "drawing/grid_paving_draw.hpp"

namespace Ariadne {

Void AugmentedPaving::draw(CanvasInterface& canvas, Projection2d const& projection) const {
    Ariadne::draw(canvas,projection,this->state_auxiliary_set());
}

} // namespace Ariadne

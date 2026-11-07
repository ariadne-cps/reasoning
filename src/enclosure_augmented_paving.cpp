// Adapters between evolved enclosures and reasoning-level grid storage.
#include "function/functional.hpp"
#include "function/constraint.hpp"
#include "set/function_set.hpp"
#include "reasoning/enclosure_augmented_paving.hpp"
#include "reasoning/hybrid_augmented_paving.hpp"
#include "evolution/hybrid_enclosure.hpp"
#include "paving/grid_paving.hpp"
#include "symbolic/space.hpp"
#include "labelled/figure.hpp"
#include "logging/logging.hpp"

namespace Ariadne {

Void adjoin_outer_approximation_to(Enclosure const& enclosure, AugmentedPaving& storage, Nat fineness)
{
    if (enclosure.auxiliary_mapping().result_size()!=0 &&
            enclosure.auxiliary_mapping().managed_pointer()==storage.auxiliary_mapping().managed_pointer())
    {
        ARIADNE_WARN("enclosure="<<enclosure<<", "
                    "enclosure.auxiliary_mapping()="<<enclosure.auxiliary_mapping()<<", "
                    "storage.auxiliary_mapping()="<<storage.auxiliary_mapping());
    }
    enclosure.configuration().paver().adjoin_outer_approximation(storage.state_paving(),enclosure.state_set(),fineness);
}


AugmentedPaving outer_approximation(Enclosure const& enclosure, Grid const& grid, Nat fineness)
{
    AugmentedPaving paving(grid,enclosure.auxiliary_mapping());
    adjoin_outer_approximation_to(enclosure,paving,fineness);
    return paving;
}

Void adjoin_outer_approximation_to(HybridEnclosure const& enclosure, HybridAugmentedPaving& storage, Nat fineness)
{
    enclosure.adjoin_outer_approximation_to(storage.state_set(),fineness);
}

} // namespace Ariadne

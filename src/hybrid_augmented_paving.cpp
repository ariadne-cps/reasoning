#include "reasoning/hybrid_augmented_paving.hpp"
#include "function/functional.hpp"

namespace Ariadne {
HybridGridTreePaving HybridAugmentedPaving::state_auxiliary_paving() const {
    HybridGrid state_grid=this->state_paving().grid();
    HybridScalings scalings=state_grid.scalings();
    HybridGrid result_grid(this->_auxiliary_mapping.state_auxiliary_space(),scalings);
    HybridGridTreePaving result(result_grid);
    for (auto it=this->state_paving().locations_begin(); it!=this->state_paving().locations_end(); ++it) {
        DiscreteLocation const& q=it->first;
        GridTreePaving const& paving=it->second;
        LabelledMapping mapping=this->_auxiliary_mapping(q);
        ARIADNE_PRECONDITION(state_grid.space(q)==mapping.argument_space());
        Grid auxiliary_grid=scalings.grid(q,mapping.result_space());
        result[q]=outer_skew_product(paving,auxiliary_grid,mapping.mapping());
    }
    return result;
}
} // namespace Ariadne

// Labelled augmented paving and labelled grid operations.
#include "reasoning/labelled_augmented_paving.hpp"

namespace Ariadne {

LabelledAugmentedPaving::LabelledAugmentedPaving(Grid const& grid,
                                 RealSpace const& state_space,
                                 EffectiveVectorMultivariateFunction const& auxiliary_mapping,
                                 RealSpace const& auxiliary_space)
    : LabelledAugmentedPaving(GridTreePaving(grid), state_space, auxiliary_mapping, auxiliary_space) { }

LabelledAugmentedPaving::LabelledAugmentedPaving(GridTreePaving const& paving,
                                 RealSpace const& state_space,
                                 EffectiveVectorMultivariateFunction const& auxiliary_mapping,
                                 RealSpace const& auxiliary_space)
    : AugmentedPaving(paving,auxiliary_mapping)
    , _state_variables(state_space.variable_names())
    , _auxiliary_variables(auxiliary_space.variable_names())
{ }


LabelledGridTreePaving inner_approximation(EffectiveEuclideanSetInterface const& set, LabelledGrid const& grid, Nat fineness) {
    LabelledGridTreePaving paving(grid); paving.euclidean_set().adjoin_inner_approximation(set,fineness); return paving;
}

const RealSpace LabelledAugmentedPaving::state_space() const {
    return RealSpace(this->_state_variables);
}

const RealSpace LabelledAugmentedPaving::auxiliary_space() const {
    return RealSpace(this->_auxiliary_variables);
}

const RealSpace LabelledAugmentedPaving::state_auxiliary_space() const {
    return join(this->state_space(),this->auxiliary_space());
}

const LabelledGrid LabelledAugmentedPaving::grid() const {
    return LabelledGrid(this->euclidean_set().grid(),this->state_space());
}

const Mapping LabelledAugmentedPaving::auxiliary_function() const {
    return this->AugmentedPaving::auxiliary_mapping();
}

const LabelledMapping LabelledAugmentedPaving::auxiliary_mapping() const {
    return LabelledMapping(this->state_space(), this->auxiliary_function(), this->auxiliary_space());
}

const LabelledMapping LabelledAugmentedPaving::auxiliary_data() const {
    return this->auxiliary_mapping();
}


} // namespace Ariadne

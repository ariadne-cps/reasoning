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
    enclosure.configuration().paver().adjoin_outer_approximation(storage.state_set(),enclosure.state_set(),fineness);
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

Void LabelledAugmentedPaving::draw(CanvasInterface& canvas, const Variables2d& axes) const
{
    Projection2d proj=projection(this->state_auxiliary_space(),axes);
    this->euclidean_set().draw(canvas,proj);
}

} // namespace Ariadne

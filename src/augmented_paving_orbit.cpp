#include "reasoning/augmented_paving_orbit.hpp"

namespace Ariadne {

Orbit<AugmentedPaving>::
Orbit(const AugmentedPaving& initial_set)
    : _data(new Orbit<AugmentedPaving>::Data(initial_set.grid(),initial_set.auxiliary_mapping()))
{
    this->_data->initial=initial_set;
}


Orbit<AugmentedPaving>::
Orbit(const AugmentedPaving& initial_set,
      const AugmentedPaving& reach_set,
      const AugmentedPaving& intermediate_set,
      const AugmentedPaving& final_set)
    : _data(new Data(initial_set.grid(),initial_set.auxiliary_mapping()))
{
    this->_data->initial=initial_set;
    this->_data->reach=reach_set;
    this->_data->intermediate=intermediate_set;
    this->_data->final=final_set;
}


AugmentedPaving const&
Orbit<AugmentedPaving>::
initial() const
{
    return this->_data->initial;
}

AugmentedPaving const&
Orbit<AugmentedPaving>::
reach() const
{
    return this->_data->reach;
}

AugmentedPaving const&
Orbit<AugmentedPaving>::
intermediate() const
{
    return this->_data->intermediate;
}

AugmentedPaving const&
Orbit<AugmentedPaving>::
final() const
{
    return this->_data->final;
}

} // namespace Ariadne

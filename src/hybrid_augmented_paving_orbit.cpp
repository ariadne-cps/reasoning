#include "reasoning/hybrid_augmented_paving_orbit.hpp"

namespace Ariadne {

struct Orbit<HybridAugmentedPaving>::Data {
    Data(const HybridGrid& grid, HybridAuxiliaryMapping const& aux)
        : initial(grid,aux), reach(grid,aux), intermediate(grid,aux), final(grid,aux) { }
    HybridAugmentedPaving initial;
    HybridAugmentedPaving reach;
    HybridAugmentedPaving intermediate;
    HybridAugmentedPaving final;
};

Orbit<HybridAugmentedPaving>::
Orbit(const HybridAugmentedPaving& initial_set)
    : _data(new Data(initial_set.grid(),initial_set.auxiliary_data()))
{
    this->_data->initial=initial_set;
}

Orbit<HybridAugmentedPaving>::
Orbit(const HybridAugmentedPaving& initial_set,
      const HybridAugmentedPaving& reach_set,
      const HybridAugmentedPaving& intermediate_set,
      const HybridAugmentedPaving& final_set)
    : _data(new Data(initial_set.grid(),initial_set.auxiliary_data()))
{
    this->_data->initial=initial_set;
    this->_data->reach=reach_set;
    this->_data->intermediate=intermediate_set;
    this->_data->final=final_set;
}

HybridAugmentedPaving const&
Orbit<HybridAugmentedPaving>::
initial() const
{
    return this->_data->initial;
}

HybridAugmentedPaving const&
Orbit<HybridAugmentedPaving>::
reach() const
{
    return this->_data->reach;
}

HybridAugmentedPaving const&
Orbit<HybridAugmentedPaving>::
intermediate() const
{
    return this->_data->intermediate;
}

HybridAugmentedPaving const&
Orbit<HybridAugmentedPaving>::
final() const
{
    return this->_data->final;
}

Void Orbit<HybridAugmentedPaving>::draw(CanvasInterface& c, const Set<DiscreteLocation>& l, const Variables2d& v) const {
    this->reach().draw(c,l,v);
}

} // namespace Ariadne

// Grid-backed storage orbits belong to reasoning, not evolution.
#ifndef ARIADNE_REASONING_AUGMENTED_PAVING_ORBIT_HPP
#define ARIADNE_REASONING_AUGMENTED_PAVING_ORBIT_HPP

#include "evolution/orbit.hpp"
#include "reasoning/augmented_paving.hpp"

namespace Ariadne {

template<>
class Orbit<AugmentedPaving>
{
    struct Data;
  public:
    typedef AugmentedPaving EnclosureListType;

    Orbit(const AugmentedPaving& initial);
    Orbit(const AugmentedPaving& initial, const AugmentedPaving& reach,
          const AugmentedPaving& intermediate, const AugmentedPaving& final);

    AugmentedPaving const& initial() const;
    AugmentedPaving const& reach() const;
    AugmentedPaving const& intermediate() const;
    AugmentedPaving const& final() const;
  private:
    std::shared_ptr<Data> _data;
};

struct Orbit<AugmentedPaving>::Data {
    Data(const Grid& grid, const EffectiveVectorMultivariateFunction& auxiliary_mapping)
            : initial(grid,auxiliary_mapping), reach(initial), intermediate(initial), final(initial) { }
    AugmentedPaving initial;
    AugmentedPaving reach;
    AugmentedPaving intermediate;
    AugmentedPaving final;
};

} // namespace Ariadne

#endif // ARIADNE_REASONING_AUGMENTED_PAVING_ORBIT_HPP

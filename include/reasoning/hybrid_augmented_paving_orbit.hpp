#ifndef ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_HPP
#define ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_HPP

#include "hybrid/hybrid_augmented_paving.hpp"
#include "evolution/hybrid_orbit.hpp"

namespace Ariadne {

template<>
class Orbit<HybridAugmentedPaving>
{
    struct Data;
  public:
    typedef HybridAugmentedPaving EnclosureListType;

    Orbit(const HybridAugmentedPaving& initial_set);
    Orbit(const HybridAugmentedPaving& initial_set, const HybridAugmentedPaving& reach_set,
          const HybridAugmentedPaving& intermediate_set, const HybridAugmentedPaving& final_set);

    HybridAugmentedPaving const& initial() const;
    HybridAugmentedPaving const& reach() const;
    HybridAugmentedPaving const& intermediate() const;
    HybridAugmentedPaving const& final() const;
  private:
    SharedPointer<Data> _data;
};

} // namespace Ariadne

#endif // ARIADNE_REASONING_HYBRID_AUGMENTED_PAVING_ORBIT_HPP

// Bridges evolution enclosures to reasoning-owned augmented pavings.
// This interface must not be declared in evolution/enclosure.hpp.
#ifndef ARIADNE_REASONING_ENCLOSURE_AUGMENTED_PAVING_HPP
#define ARIADNE_REASONING_ENCLOSURE_AUGMENTED_PAVING_HPP

#include "reasoning/augmented_paving.hpp"
#include "evolution/enclosure.hpp"

namespace Ariadne {

// Adjoin the outer approximation of an evolved enclosure to analysis augmented paving.
Void adjoin_outer_approximation_to(Enclosure const& enclosure, AugmentedPaving& storage, Nat fineness);

// Construct analysis augmented paving containing an outer approximation of the enclosure.
AugmentedPaving outer_approximation(Enclosure const& enclosure, Grid const& grid, Nat fineness);

class HybridEnclosure;
class HybridAugmentedPaving;

// Preserve the hybrid paving semantics without exposing reasoning AugmentedPaving in evolution.
Void adjoin_outer_approximation_to(HybridEnclosure const& enclosure, HybridAugmentedPaving& storage, Nat fineness);

} // namespace Ariadne

#endif // ARIADNE_REASONING_ENCLOSURE_AUGMENTED_PAVING_HPP

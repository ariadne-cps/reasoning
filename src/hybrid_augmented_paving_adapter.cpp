/***************************************************************************
 *            hybrid_augmented_paving_adapter.cpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne Reasoning.
 *
 *  Ariadne Reasoning is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne Reasoning is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne Reasoning. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file src/hybrid_augmented_paving_adapter.cpp
 *  @brief Adapt hybrid automata into location-dependent auxiliary mappings.
 */

#include <memory>
#include <utility>

#include "reasoning/analysis_paving_factory.hpp"
#include "system/hybrid_automaton_interface.hpp"

namespace Ariadne {
namespace {
class OwnedSystemAuxiliarySpace : public HybridSpaceInterface {
    std::shared_ptr<const HybridAutomatonInterface> _system;

  public:
    explicit OwnedSystemAuxiliarySpace(std::shared_ptr<const HybridAutomatonInterface> system)
        : _system(std::move(system)) {}
    OwnedSystemAuxiliarySpace* clone() const override { return new OwnedSystemAuxiliarySpace(*this); }
    Bool has_location(const DiscreteLocation& q) const override { return _system->has_mode(q); }
    RealSpace operator[](const DiscreteLocation& q) const override {
        return _system->state_auxiliary_space()[q];
    }
    OutputStream& _write(OutputStream& os) const override {
        return os << "OwnedSystemAuxiliarySpace(" << _system->name() << ")";
    }
    ValidatedKleenean operator==(const HybridSpaceInterface& other) const override {
        const auto* rhs = dynamic_cast<const OwnedSystemAuxiliarySpace*>(&other);
        if (!rhs)
            return indeterminate;
        if (_system.get() == rhs->_system.get())
            return true;
        return indeterminate;
    }
};
} // namespace
HybridAuxiliaryMapping make_hybrid_auxiliary_mapping(const HybridAutomatonInterface& system) {
    auto owned = std::shared_ptr<const HybridAutomatonInterface>(system.clone());
    HybridSpace auxiliary_space(new OwnedSystemAuxiliarySpace(owned));
    return HybridAuxiliaryMapping(auxiliary_space,
                                  [owned](const DiscreteLocation& q) {
                                      return LabelledMapping(owned->continuous_state_space(q),
                                                             owned->auxiliary_function(q),
                                                             owned->continuous_auxiliary_space(q));
                                  });
}
} // namespace Ariadne

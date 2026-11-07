#include <memory>
#include <utility>
#include "reasoning/analysis_paving_factory.hpp"
#include "system/hybrid_automaton_interface.hpp"

namespace Ariadne {
namespace {
class OwnedSystemAuxiliarySpace : public HybridSpaceInterface {
    std::shared_ptr<HybridAutomatonInterface const> _system;
  public:
    explicit OwnedSystemAuxiliarySpace(std::shared_ptr<HybridAutomatonInterface const> system)
        : _system(std::move(system)) { }
    OwnedSystemAuxiliarySpace* clone() const override { return new OwnedSystemAuxiliarySpace(*this); }
    Bool has_location(DiscreteLocation const& q) const override { return _system->has_mode(q); }
    RealSpace operator[](DiscreteLocation const& q) const override {
        return _system->state_auxiliary_space()[q];
    }
    OutputStream& _write(OutputStream& os) const override {
        return os << "OwnedSystemAuxiliarySpace(" << _system->name() << ")";
    }
    ValidatedKleenean operator==(HybridSpaceInterface const& other) const override {
        auto const* rhs=dynamic_cast<OwnedSystemAuxiliarySpace const*>(&other);
        if (!rhs) return indeterminate;
        if (_system.get()==rhs->_system.get()) return true;
        return indeterminate;
    }
};
} // namespace
HybridAuxiliaryMapping make_hybrid_auxiliary_mapping(HybridAutomatonInterface const& system) {
    auto owned=std::shared_ptr<HybridAutomatonInterface const>(system.clone());
    HybridSpace auxiliary_space(new OwnedSystemAuxiliarySpace(owned));
    return HybridAuxiliaryMapping(auxiliary_space,
        [owned](DiscreteLocation const& q) {
            return LabelledMapping(owned->continuous_state_space(q),
                                   owned->auxiliary_function(q),
                                   owned->continuous_auxiliary_space(q));
        });
}
} // namespace Ariadne

/***************************************************************************
 *            hybrid/hybrid_augmented_paving.hpp
 *
 *  Copyright  2009-20  Pieter Collins
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne.
 *
 *  Ariadne is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne.  If not, see <https://www.gnu.org/licenses/>.
 */

/*! \file reasoning/hybrid_augmented_paving.hpp
 *  \brief Grid-tree pavings augmented with location-dependent auxiliary functions
 */

#ifndef ARIADNE_HYBRID_AUGMENTED_PAVING_HPP
#define ARIADNE_HYBRID_AUGMENTED_PAVING_HPP

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <functional>
#include <utility>

#include "logging/logging.hpp"
#include "numeric/declarations.hpp"
#include "algebra/declarations.hpp"
#include "function/declarations.hpp"
#include "geometry/declarations.hpp"
#include "utility/pointer.hpp"
#include "utility/container.hpp"

#include "hybrid/hybrid_set.decl.hpp"
#include "hybrid/discrete_location.hpp"
#include "hybrid/hybrid_graphics_interface.hpp"

#include "geometry/box.hpp"
// HybridAugmentedPaving owns a HybridGridTreePaving by value, so the type must be complete.
#include "hybrid/hybrid_paving.hpp"
#include "reasoning/augmented_paving.hpp"


namespace Ariadne {

template<class X> class Vector;

class AugmentedPaving;
class GridTreePaving;
class HybridAugmentedPaving;

//! Location-indexed auxiliary functions, evaluated lazily and independent of an automaton.
class HybridAuxiliaryMapping {
    // Destroy the hybrid space before the provider, which may own its backing system.
    std::function<LabelledMapping(DiscreteLocation const&)> _mapping;
    HybridSpace _state_auxiliary_space;
  public:
    HybridAuxiliaryMapping(HybridSpace const& space,
                           std::function<LabelledMapping(DiscreteLocation const&)> mapping)
        : _mapping(std::move(mapping)), _state_auxiliary_space(space) { }
    LabelledMapping operator()(DiscreteLocation const& q) const { return _mapping(q); }
    HybridSpace state_auxiliary_space() const { return _state_auxiliary_space; }
};

// FIXME: Replace this proxy with a direct mutable view of a labelled augmented paving.
class HybridAugmentedPavingElementReference {
    HybridAugmentedPaving* _hstorage; DiscreteLocation _loc;
  public:
    HybridAugmentedPavingElementReference(HybridAugmentedPaving* hstorage, DiscreteLocation const& loc) : _hstorage(hstorage), _loc(loc) { }
    inline operator const LabelledAugmentedPaving () const;
    inline Void clear();
};

//! \ingroup HybridSetSubModule
//! \brief A hybrid paving with a location-indexed algebraic auxiliary mapping.
class HybridAugmentedPaving
    : public HybridDrawableInterface
{
    HybridGridTreePaving  _state_set;
    HybridAuxiliaryMapping _auxiliary_mapping;
  public:
    typedef HybridGrid GridType;
    typedef HybridGridTreePaving PavingType;

    HybridAugmentedPaving(HybridGrid const& hg, HybridAuxiliaryMapping const& aux)
        : _state_set(hg), _auxiliary_mapping(aux) { }
    HybridAugmentedPaving(HybridGridTreePaving const& hgtp, HybridAuxiliaryMapping const& aux)
        : _state_set(hgtp), _auxiliary_mapping(aux) { }

    HybridAuxiliaryMapping const& auxiliary_data() const { return _auxiliary_mapping; }
    LabelledMapping auxiliary_mapping(DiscreteLocation const& loc) const {
        return _auxiliary_mapping(loc); }

    HybridGrid grid() const { return this->_state_set.grid(); }
    HybridGridTreePaving& state_set() {
        return this->_state_set; }
    HybridGridTreePaving const& state_set() const {
        return this->_state_set; }
    HybridGridTreePaving state_auxiliary_paving() const;

    Bool is_empty() const { return this->_state_set.is_empty(); }
    SizeType size() const { return this->_state_set.size(); }
    HybridGridTreePaving::ConstIterator begin() const { return this->_state_set.begin(); }
    HybridGridTreePaving::ConstIterator end() const { return this->_state_set.end(); }

    Void clear() { this->_state_set.clear(); }
    Void adjoin(HybridAugmentedPaving const& other) { return this->_state_set.adjoin(other._state_set); }
    Void remove(HybridAugmentedPaving const& other) { return this->_state_set.remove(other._state_set); }
    Void restrict(HybridAugmentedPaving const& other) { return this->_state_set.restrict(other._state_set); }

    friend Bool subset(HybridAugmentedPaving const& set1, HybridAugmentedPaving const& set2) {
        return subset(set1._state_set,set2._state_set); }
    friend Bool intersect(HybridAugmentedPaving const& set1, HybridAugmentedPaving const& set2) {
        return intersect(set1._state_set,set2._state_set); }

    Void adjoin_lower_approximation(EffectiveHybridOvertSetInterface const& set, Nat extent, Nat fineness) {
        this->_state_set.adjoin_lower_approximation(set,extent,fineness); }
    Void adjoin_outer_approximation(EffectiveHybridCompactSetInterface const& set, Nat fineness) {
        this->_state_set.adjoin_outer_approximation(set,fineness); }
    Void adjoin_outer_approximation(HybridExactBoxes const& bx, Nat fineness) {
        this->_state_set.adjoin_outer_approximation(bx,fineness); }

    Void restrict_to_extent(Nat level) { this->_state_set.restrict_to_extent(level); }
    Void mince(Nat fineness) { this->_state_set.mince(fineness); }
    Void recombine() { this->_state_set.recombine(); }

    const LabelledAugmentedPaving operator[] (DiscreteLocation const& loc) const;
    HybridAugmentedPavingElementReference operator[] (DiscreteLocation const& loc) {
        return HybridAugmentedPavingElementReference{this,loc}; }

    virtual Void draw(CanvasInterface& cnv, const Set<DiscreteLocation>& locs, const Variables2d& vars) const override {
        this->state_auxiliary_paving().draw(cnv,locs,vars);
    }

    friend OutputStream& operator<<(OutputStream& os, HybridAugmentedPaving const& set) {
        return os << "HybridAugmentedPaving(" << set._state_set << ", state_auxiliary_space="
                  << set._auxiliary_mapping.state_auxiliary_space() << ")"; }

    friend class HybridAugmentedPavingElementReference;
};

inline const LabelledAugmentedPaving HybridAugmentedPaving::operator[](DiscreteLocation const& q) const {
    return LabelledAugmentedPaving(this->_state_set[q],this->auxiliary_mapping(q));
}
HybridAugmentedPavingElementReference::operator const LabelledAugmentedPaving () const {
    return LabelledAugmentedPaving(_hstorage->_state_set[_loc],_hstorage->auxiliary_mapping(_loc)); }
Void HybridAugmentedPavingElementReference::clear() { return _hstorage->state_set()[_loc].clear(); }

} // namespace Ariadne

#endif // ARIADNE_HYBRID_AUGMENTED_PAVING_HPP

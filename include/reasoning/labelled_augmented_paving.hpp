/***************************************************************************
 *            reasoning/labelled_augmented_paving.hpp
 *
 *  Copyright  2011-20  Pieter Collins
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

/*! \file reasoning/labelled_augmented_paving.hpp
 *  \brief A labelled paving augmented with algebraic auxiliary mappings.
 */
#ifndef ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_HPP
#define ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_HPP
#include "reasoning/augmented_paving.hpp"
#include "reasoning/labelled_grid.hpp"
#include "reasoning/labelled_mapping.hpp"

namespace Ariadne {

class LabelledAugmentedPaving
    : public AugmentedPaving
{
    template<class SYS> static LabelledMapping labelled_auxiliary_mapping(SYS const& sys) {
        return LabelledMapping(sys.state_space(),sys.auxiliary_mapping(),sys.auxiliary_space()); }
  public:
    typedef LabelledGrid GridType;

    LabelledAugmentedPaving(Grid const& grid, RealSpace const& state_space,
                    Mapping const& auxiliary_mapping, RealSpace const& auxiliary_space);
    LabelledAugmentedPaving(GridTreePaving const& paving, RealSpace const& state_space,
                    Mapping const& auxiliary_mapping, RealSpace const& auxiliary_space);

    LabelledAugmentedPaving(Grid const& grid, LabelledMapping const& auxiliary)
        : LabelledAugmentedPaving(GridTreePaving(grid),auxiliary) { }
    LabelledAugmentedPaving(GridTreePaving const& paving, LabelledMapping const& auxiliary)
        : LabelledAugmentedPaving(paving,auxiliary.argument_space(),auxiliary.mapping(),auxiliary.result_space()) {
            ARIADNE_PRECONDITION(paving.dimension()==auxiliary.argument_space().dimension()); }
    LabelledAugmentedPaving(LabelledGrid const& grid, LabelledMapping const& auxiliary)
        : LabelledAugmentedPaving(grid.euclidean_grid(),auxiliary) { ARIADNE_PRECONDITION(grid.space()==auxiliary.argument_space()); }
    LabelledAugmentedPaving(LabelledGridTreePaving const& paving, LabelledMapping const& auxiliary)
        : LabelledAugmentedPaving(paving.euclidean_set(),auxiliary) { ARIADNE_PRECONDITION(paving.space()==auxiliary.argument_space()); }

    template<class SYS> LabelledAugmentedPaving(Grid const& grid, SYS const& system)
        : LabelledAugmentedPaving(grid,system.state_space(),system.auxiliary_mapping(),system.auxiliary_space()) { }
    template<class SYS> LabelledAugmentedPaving(GridTreePaving const& paving, SYS const& system)
        : LabelledAugmentedPaving(paving,system.state_space(),system.auxiliary_mapping(),system.auxiliary_space()) { }
    template<class SYS> LabelledAugmentedPaving(LabelledGrid const& grid, SYS const& system)
        : LabelledAugmentedPaving(grid.euclidean_grid(),system.state_space(),system.auxiliary_mapping(),system.auxiliary_space()) {
            ARIADNE_PRECONDITION(grid.space()==system.state_space()); }
    template<class SYS> LabelledAugmentedPaving(LabelledGridTreePaving const& paving, SYS const& system)
        : LabelledAugmentedPaving(paving.euclidean_set(),system.state_space(),system.auxiliary_mapping(),system.auxiliary_space()) {
            ARIADNE_PRECONDITION(paving.space()==system.state_space()); }

    AugmentedPaving& euclidean_set() { return *this; }
    AugmentedPaving const& euclidean_set() const { return *this; }
    const RealSpace state_space() const;
    const RealSpace auxiliary_space() const;
    const RealSpace state_auxiliary_space() const;

    const LabelledGrid grid() const;
    const LabelledMapping auxiliary_data() const;
    const LabelledMapping auxiliary_mapping() const;
    const Mapping auxiliary_function() const;

    Void adjoin(LabelledAugmentedPaving const& other) {
        ARIADNE_PRECONDITION(this->_state_variables==other._state_variables);
        this->state_set().adjoin(other.state_set()); }
    Void restrict(LabelledAugmentedPaving const& other) {
        ARIADNE_PRECONDITION(this->_state_variables==other._state_variables);
        this->state_set().restrict(other.state_set()); }
    Void remove(LabelledAugmentedPaving const& other) {
        ARIADNE_PRECONDITION(this->_state_variables==other._state_variables);
        this->state_set().remove(other.state_set()); }

    friend Bool subset(LabelledAugmentedPaving const& set1, LabelledAugmentedPaving const& set2) {
        ARIADNE_PRECONDITION(set1._state_variables==set2._state_variables);
        return subset(set1.state_set(),set2.state_set()); }
    friend Bool intersect(LabelledAugmentedPaving const& set1, LabelledAugmentedPaving const& set2) {
        ARIADNE_PRECONDITION(set1._state_variables==set2._state_variables);
        return intersect(set1.state_set(),set2.state_set()); }

    virtual LabelledAugmentedPaving* clone() const override { return new LabelledAugmentedPaving(*this); }

    friend OutputStream& operator<<(OutputStream& os, LabelledAugmentedPaving const& set) {
        return os << "LabelledAugmentedPaving(" << set.state_set() << ", state_variables=" << set._state_variables << ", auxiliary_mapping=" << set.auxiliary_mapping().function() << ", auxiliary_variables=" << set._auxiliary_variables << ")"; }

  private:
    List<Identifier> _state_variables;
    List<Identifier> _auxiliary_variables;
};

LabelledGridTreePaving inner_approximation(EffectiveEuclideanSetInterface const& set, LabelledGrid const& grid, Nat fineness);

} // namespace Ariadne
#endif // ARIADNE_REASONING_LABELLED_AUGMENTED_PAVING_HPP

/***************************************************************************
 *            dynamics/augmented_paving.hpp
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

/*! \file reasoning/augmented_paving.hpp
 *  \brief Paved sets for continuous systems
 */

#ifndef ARIADNE_AUGMENTED_PAVING_HPP
#define ARIADNE_AUGMENTED_PAVING_HPP

#include <iosfwd>
#include "function/function.hpp"
#include "geometry/set_interface.hpp"
#include "paving/paving_interface.hpp"
#include "paving/grid_paving.hpp"
#include "symbolic/identifier.hpp"
#include "symbolic/space.hpp"
#include "labelled/graphics_interface.hpp"
#include "drawing/grid_paving_draw.hpp"

namespace Ariadne {

typedef EffectiveVectorMultivariateFunction Mapping;

//! \ingroup DynamicsModule
//! \brief A grid-tree paving of states equipped with a function defining auxiliary variables.
class AugmentedPaving
    : public Drawable2dInterface
{
    GridTreePaving _state_set;
    EffectiveVectorMultivariateFunction _auxiliary_mapping;
  public:
    typedef Grid GridType;
    typedef GridTreePaving PavingType;
//    AugmentedPaving(Grid const& g) : _state_set(g), _auxiliary_mapping(0,g.dimension()) { }
//    AugmentedPaving(GridTreePaving const& gtp) : _state_set(gtp), _auxiliary_mapping(0,gtp.dimension()) { }
    AugmentedPaving(Grid const& g, Mapping const& aux)
        : _state_set(g), _auxiliary_mapping(aux) { }
    AugmentedPaving(GridTreePaving const& gtp, Mapping const& aux)
        : _state_set(gtp), _auxiliary_mapping(aux) { }

    GridTreePaving& state_set() { return this->_state_set; }
    GridTreePaving const& state_set() const { return this->_state_set; }
    GridTreePaving state_auxiliary_set() const {
        Grid auxiliary_grid(this->_auxiliary_mapping.result_size());
        return outer_skew_product(this->_state_set, auxiliary_grid, this->_auxiliary_mapping); }
    Mapping const& auxiliary_mapping() const { return this->_auxiliary_mapping; }
    Mapping const& auxiliary_function() const { return this->_auxiliary_mapping; }
    Mapping const& auxiliary_data() const { return this->_auxiliary_mapping; }

    Bool is_empty() const { return this->_state_set.is_empty(); }
    SizeType size() const { return this->_state_set.size(); }
    GridTreePaving::ConstIterator begin() const { return this->_state_set.begin(); }
    GridTreePaving::ConstIterator end() const { return this->_state_set.end(); }

    Grid grid() const { return this->_state_set.grid(); }

    Void clear() { this->_state_set.clear(); }
    Void adjoin(AugmentedPaving const& other) { return this->_state_set.adjoin(other._state_set); }
    Void restrict(AugmentedPaving const& other) { return this->_state_set.restrict(other._state_set); }
    Void remove(AugmentedPaving const& other) { return this->_state_set.remove(other._state_set); }

    Void adjoin_inner_approximation(EffectiveEuclideanSetInterface const& set, Nat fineness) {
        this->_state_set.adjoin_inner_approximation(set,fineness); }
    Void adjoin_lower_approximation(EffectiveEuclideanOvertSetInterface const& set, Nat extent, Nat fineness) {
        this->_state_set.adjoin_lower_approximation(set,extent,fineness); }
    Void adjoin_outer_approximation(EffectiveEuclideanCompactSetInterface const& set, Nat fineness) {
        this->_state_set.adjoin_outer_approximation(set,fineness); }
    Void adjoin_outer_approximation(ValidatedEuclideanCompactSetInterface const& set, Nat fineness) {
        this->_state_set.adjoin_outer_approximation(set,fineness); }
    Void adjoin_outer_approximation(ExactBoxType const& bx, Nat fineness) {
        this->_state_set.adjoin_outer_approximation(bx,fineness); }

    Void restrict_to_extent(Nat level) { this->_state_set.restrict_to_extent(level); }
    Void mince(Nat fineness) { this->_state_set.mince(fineness); }
    Void recombine() { this->_state_set.recombine(); }

    friend Bool subset(AugmentedPaving const& set1, AugmentedPaving const& set2) {
        //ARIADNE_PRECONDITION(same(set1._auxiliary_mapping,set2._auxiliary_mapping));
        return subset(set1._state_set,set2._state_set); }
    friend Bool intersect(AugmentedPaving const& set1, AugmentedPaving const& set2) {
        return intersect(set1._state_set,set2._state_set); }

    virtual AugmentedPaving* clone() const override { return new AugmentedPaving(*this); }
    virtual DimensionType dimension() const override { return this->_state_set.dimension(); }
    virtual Void draw(CanvasInterface& c, const Projection2d& p) const override {
        Ariadne::draw(c,p,this->state_auxiliary_set()); }

    friend OutputStream& operator<<(OutputStream& os, AugmentedPaving const& set) {
        return os << "AugmentedPaving(" << set._state_set << "," << set._auxiliary_mapping << ")"; }
};


struct LabelledGrid {
    template<class SYS> explicit LabelledGrid(SYS const& system)
        : LabelledGrid(Grid(system.state_space().dimension()),system.state_space()) { }
    LabelledGrid(Grid const& grid, RealSpace const& space)
        : _grid(grid), _variables(space.variable_names()) { }

    Grid const& euclidean_grid() const { return this->_grid; }
    const RealSpace space() const { return RealSpace(this->_variables); }

    friend OutputStream& operator<<(OutputStream& os, LabelledGrid const& grid) {
        return os << "LabelledGrid( " << grid._grid << ", " << grid._variables << ")"; }
  private:
    Grid _grid;
    List<Identifier> _variables;
};


struct LabelledGridTreePaving {
  public:
    LabelledGridTreePaving(Grid const& grid, RealSpace const& space)
        : _set(grid), _variables(space.variable_names()) { }
    LabelledGridTreePaving(GridTreePaving set, RealSpace const& space)
        : _set(set), _variables(space.variable_names()) { }
    explicit LabelledGridTreePaving(LabelledGrid const& lgrid)
        : LabelledGridTreePaving(lgrid.euclidean_grid(),lgrid.space()) { }
    GridTreePaving& euclidean_set() { return this->_set; }
    GridTreePaving const& euclidean_set() const { return this->_set; }
    const RealSpace space() const { return RealSpace(this->_variables); }
  private:
    GridTreePaving _set;
    List<Identifier> _variables;
};


struct LabelledStateAuxiliaryGrid {
    template<class SYS> explicit LabelledStateAuxiliaryGrid(SYS const& system)
        : LabelledStateAuxiliaryGrid(Grid(system.state_space().dimension()),system.state_space(),
                       system.auxiliary_mapping(),system.auxiliary_space()) { }
    LabelledStateAuxiliaryGrid(Grid const& grid, RealSpace const& state_space);
    LabelledStateAuxiliaryGrid(Grid const& grid, RealSpace const& state_space,
                 EffectiveVectorMultivariateFunction const& auxiliary_mapping, RealSpace const& auxiliary_space)
        : _grid(grid), _auxiliary_mapping(auxiliary_mapping)
        , _state_variables(state_space.variable_names()),  _auxiliary_variables(auxiliary_space.variable_names()) { }

    Grid const& euclidean_grid() const { return this->_grid; }
    EffectiveVectorMultivariateFunction const& auxiliary_mapping() const { return this->_auxiliary_mapping; }
    const RealSpace state_space() const { return RealSpace(this->_state_variables); }
    const RealSpace auxiliary_space() const { return RealSpace(this->_auxiliary_variables); }

    friend OutputStream& operator<<(OutputStream& os, LabelledStateAuxiliaryGrid const& grid) {
        return os << "LabelledStateAuxiliaryGrid( " << grid._grid << ", " << grid._state_variables << ", "
                  << grid._auxiliary_mapping << ", " << grid._auxiliary_variables << ")"; }
  private:
    Grid _grid;
    EffectiveVectorMultivariateFunction _auxiliary_mapping;
    List<Identifier> _state_variables;
    List<Identifier> _auxiliary_variables;
};

class LabelledMapping {
  public:
    LabelledMapping(RealSpace argument_space, Mapping mapping, RealSpace result_space)
        : _mapping(mapping), _argument_variables(argument_space.variable_names()), _result_variables(result_space.variable_names()) { }

    const RealSpace argument_space() const { return RealSpace(this->_argument_variables); }
    const Mapping& mapping() const { return this->_mapping; }
    const Mapping& function() const { return this->_mapping; }
    const RealSpace result_space() const { return RealSpace(this->_result_variables); }
  private:
    Mapping _mapping;
    List<Identifier> _argument_variables;
    List<Identifier> _result_variables;
};

class LabelledAugmentedPaving
    : public LabelledDrawable2dInterface, public AugmentedPaving
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
    virtual LabelledAugmentedPaving* labelled_clone() const override { return clone(); }

    using AugmentedPaving::draw;
    virtual Void draw(CanvasInterface& c, const Variables2d& p) const override;

    friend OutputStream& operator<<(OutputStream& os, LabelledAugmentedPaving const& set) {
        return os << "LabelledAugmentedPaving(" << set.state_set() << ", state_variables=" << set._state_variables << ", auxiliary_mapping=" << set.auxiliary_mapping().function() << ", auxiliary_variables=" << set._auxiliary_variables << ")"; }

  private:
    List<Identifier> _state_variables;
    List<Identifier> _auxiliary_variables;
};

LabelledGridTreePaving inner_approximation(EffectiveEuclideanSetInterface const& set, LabelledGrid const& grid, Nat fineness);

} //namespace Ariadne

#endif /* ARIADNE_AUGMENTED_PAVING_HPP */

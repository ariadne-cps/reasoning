/***************************************************************************
 *            reasoning/labelled_grid.hpp
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

/*! \file reasoning/labelled_grid.hpp
 *  \brief Grids, pavings and state/auxiliary grids with named variables.
 */
#ifndef ARIADNE_REASONING_LABELLED_GRID_HPP
#define ARIADNE_REASONING_LABELLED_GRID_HPP
#include "paving/grid_paving.hpp"
#include "symbolic/identifier.hpp"
#include "symbolic/space.hpp"
#include "function/function.hpp"
#include "utility/container.hpp"

namespace Ariadne {

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

} // namespace Ariadne
#endif // ARIADNE_REASONING_LABELLED_GRID_HPP

/***************************************************************************
 *            include/reasoning/neural_symbolic.hpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne Reasoning.
 *
 *  Ariadne Reasoning is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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

/** @file reasoning/neural_symbolic.hpp
 *  @brief Expansion of frozen dense networks into Ariadne real expressions.
 */
#ifndef ARIADNE_REASONING_NEURAL_SYMBOLIC_HPP
#define ARIADNE_REASONING_NEURAL_SYMBOLIC_HPP

#include "ann/network.hpp"
#include "symbolic/expression.hpp"

#include <vector>

namespace Ariadne {

/** @brief Symbolically expand a frozen neural network.
 *
 *  Each binary64 parameter is embedded as its exact dyadic Real value,
 *  and every supported layer becomes an Expression<Real> DAG.
 *  Identity, ReLU, Tanh, Sigmoid and Softplus use Symbolic operations.
 *  Symbolic's sigmoid/softplus are expression-level compositions of
 *  existing real elementary operators, not duplicated numeric kernels.
 *  Validated numeric evaluation uses Numeric's existing implementations.
 *  This expansion preserves mathematical real-function semantics, NOT
 *  finite-precision CPU floating-point evaluation order. It may be large.
 *
 *  The variable list must match the network input dimension and contain
 *  distinct variables; derivative() and simplify() can subsequently be
 *  applied to smooth expression regions using Symbolic.
 */
std::vector<Expression<Real>> make_expression(
    NeuralParameterSnapshot const& snapshot,
    std::vector<Variable<Real>> const& input_variables);

} // namespace Ariadne

#endif // ARIADNE_REASONING_NEURAL_SYMBOLIC_HPP

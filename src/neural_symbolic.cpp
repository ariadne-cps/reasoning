/***************************************************************************
 *            neural_symbolic.cpp
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

/** @file src/neural_symbolic.cpp
 *  @brief Implement symbolic expansion of frozen neural networks.
 */

#include "reasoning/neural_symbolic.hpp"
#include "numeric/builtin.hpp"
#include "numeric/real.hpp"

#include <stdexcept>
#include <utility>
#include <vector>

namespace Ariadne {
namespace {

RealExpression exact_real(double x) {
    // Real(double) is intentionally deleted: the binary64 payload must
    // explicitly be interpreted as its exact dyadic real number.
    return RealExpression(Real(ExactDouble(x)));
}

RealExpression activate(RealExpression value, NeuralActivation activation) {
    switch (activation) {
        case NeuralActivation::Relu: return max(value, RealExpression(0));
        case NeuralActivation::Tanh: return tanh(value);
        case NeuralActivation::Sigmoid: return sigmoid(value);
        case NeuralActivation::Softplus: return softplus(value);
        // NeuralNetwork validates activations at construction and exposes
        // immutable layouts, so Identity is the only remaining activation.
        default: return value;
    }
}

} // namespace

std::vector<Expression<Real>> make_expression(
    NeuralParameterSnapshot const& snapshot,
    Space<Real> const& space) {
    auto const& network=snapshot.network();
    if (space.size()!=network.input_dimension())
        throw std::invalid_argument("Wrong dimension of neural input space");

    std::vector<RealExpression> state;
    state.reserve(space.size());
    for (SizeType i=0; i<space.size(); ++i)
        state.emplace_back(space.variable(i));

    auto const& weights=snapshot.values();
    for (auto const& layer : network.layers()) {
        std::vector<RealExpression> output;
        output.reserve(layer.outputs);
        for (std::size_t o=0;o<layer.outputs;++o) {
            auto sum=exact_real(weights[layer.biases_offset+o]);
            for (std::size_t i=0;i<layer.inputs;++i)
                sum+=exact_real(weights[layer.weights_offset+o*layer.inputs+i])
                    * state[i];
            output.push_back(activate(std::move(sum),layer.activation));
        }
        state.swap(output);
    }
    return state;
}

} // namespace Ariadne

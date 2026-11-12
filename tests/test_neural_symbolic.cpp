/***************************************************************************
 *            test_neural_symbolic.cpp
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

/** @file tests/test_neural_symbolic.cpp
 *  @brief Test neural symbolic semantics and regressions.
 */

#include <cmath>
#include <stdexcept>
#include <vector>

#include "numeric/builtin.hpp"
#include "numeric/real.hpp"

#include "ariadne-reasoning.hpp"
#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestSymbolic : public Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    void test_affine_exact_coefficients_and_derivatives() {
        RealVariable x("x"), y("y");
        NeuralParameterSnapshot model(
            NeuralNetwork(2, {
                                 {1, NeuralActivation::Identity}
        }),
            {2.0, -3.0, 0.5});
        const auto expr = make_expression(model, RealSpace({x, y}));
        ARIADNE_TESTING_EXPECT_EQUAL(expr.size(), std::size_t{1});
        // Mathematical real arithmetic; both derivatives are constant.
        ARIADNE_TESTING_PROVE(identical(
            simplify(derivative(expr[0], x)), RealExpression(2)));
        ARIADNE_TESTING_PROVE(identical(
            simplify(derivative(expr[0], y)), RealExpression(-3)));
        Map<Identifier, Real> point;
        point[x.name()] = Real(1);
        point[y.name()] = Real(2);
        // Real evaluation is lazy: compare certified numerical bounds,
        // not the syntax of an unevaluated Real arithmetic expression.
        const FloatDPBounds value = evaluate(expr[0], point).get(dp);
        ARIADNE_TESTING_PROVE(value.lower_raw().get_d() <= -3.5);
        ARIADNE_TESTING_PROVE(value.upper_raw().get_d() >= -3.5);
        ARIADNE_TESTING_PROVE(value.lower_raw().get_d() > -3.500000000001);
        ARIADNE_TESTING_PROVE(value.upper_raw().get_d() < -3.499999999999);
        ARIADNE_TESTING_EXPECT_THROWS(std::invalid_argument,
                                      (void)make_expression(model, RealSpace({x})));
        // The argument space, rather than a raw variable list, defines
        // how input coordinates correspond to named variables.
        const auto reversed = make_expression(model, RealSpace({y, x}));
        ARIADNE_TESTING_PROVE(identical(
            simplify(derivative(reversed[0], x)), RealExpression(-3)));
        ARIADNE_TESTING_PROVE(identical(
            simplify(derivative(reversed[0], y)), RealExpression(2)));
    }

    void test_supported_nonlinear_activations() {
        RealVariable x("x");
        for (auto activation: {NeuralActivation::Identity,
                               NeuralActivation::Relu,
                               NeuralActivation::Tanh,
                               NeuralActivation::Sigmoid,
                               NeuralActivation::Softplus}) {
            NeuralParameterSnapshot model(
                NeuralNetwork(1, {
                                     {1, activation}
            }),
                {1.0, 0.0});
            const auto expressions = make_expression(model, RealSpace({x}));
            ARIADNE_TESTING_EXPECT_EQUAL(expressions.size(), std::size_t{1});
            ARIADNE_TESTING_PROVE(expressions[0].kind() != OperatorKind::NULLARY);

            // Independent expression-level reference without the affine layer.
            // Algebraically 0 + 1*x is x, but 'identical' tests tree identity.
            RealExpression expected(x);
            switch (activation) {
                case NeuralActivation::Identity:
                    break;
                case NeuralActivation::Relu:
                    expected = max(expected, RealExpression(0));
                    break;
                case NeuralActivation::Tanh:
                    expected = tanh(expected);
                    break;
                case NeuralActivation::Sigmoid:
                    expected = sigmoid(expected);
                    break;
                case NeuralActivation::Softplus:
                    expected = softplus(expected);
                    break;
                default:
                    throw std::invalid_argument("Unsupported test activation");
            }

            // Check semantic equivalence on both sides of the ReLU kink,
            // including zero, using the rigorous Real-to-Bounds evaluation.
            for (double input: {-2.0, -0.5, 0.0, 0.5, 2.0}) {
                Map<Identifier, Real> point;
                point[x.name()] = Real(ExactDouble(input));
                const FloatDPBounds actual = evaluate(expressions[0], point).get(dp);
                const FloatDPBounds reference = evaluate(expected, point).get(dp);
                ARIADNE_TESTING_PROVE(
                    actual.lower_raw().get_d() <= reference.upper_raw().get_d());
                ARIADNE_TESTING_PROVE(
                    actual.upper_raw().get_d() >= reference.lower_raw().get_d());
                ARIADNE_TESTING_PROVE(
                    std::abs(actual.lower_raw().get_d() - reference.lower_raw().get_d()) < 1e-12);
                ARIADNE_TESTING_PROVE(
                    std::abs(actual.upper_raw().get_d() - reference.upper_raw().get_d()) < 1e-12);
            }

            // Differentiation remains tested for the two smooth activations;
            // no derivative is assumed at the ReLU kink.
            if (activation == NeuralActivation::Sigmoid ||
                activation == NeuralActivation::Softplus) {
                for (double input: {-2.0, -0.5, 0.0, 0.5, 2.0}) {
                    Map<Identifier, Real> point;
                    point[x.name()] = Real(ExactDouble(input));
                    const FloatDPBounds actual_derivative =
                        evaluate(derivative(expressions[0], x), point).get(dp);
                    const FloatDPBounds reference_derivative =
                        evaluate(derivative(expected, x), point).get(dp);
                    ARIADNE_TESTING_PROVE(std::abs(
                                              actual_derivative.lower_raw().get_d() -
                                              reference_derivative.lower_raw().get_d()) < 1e-12);
                    ARIADNE_TESTING_PROVE(std::abs(
                                              actual_derivative.upper_raw().get_d() -
                                              reference_derivative.upper_raw().get_d()) < 1e-12);
                }
            }
        }

        ARIADNE_TESTING_PROVE(
            identical(simplify(derivative(tanh(RealExpression(x)), x)),
                      1 - sqr(tanh(RealExpression(x)))));
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("symbolic");
    ARIADNE_TESTING_ADD_TEST(TestSymbolic, test_affine_exact_coefficients_and_derivatives);
    ARIADNE_TESTING_ADD_TEST(TestSymbolic, test_supported_nonlinear_activations);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}

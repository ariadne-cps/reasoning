/***************************************************************************
 *            test_analyser_logging_hybrid.cpp
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

/** @file tests/test_analyser_logging_hybrid.cpp
 *  @brief Hybrid reachability analyser logging tests.
 */

#include "analyser_logging_common.hpp"

class TestAnalyserLoggingHybrid : public TestAnalyserLoggingBase {
  public:
    using TestAnalyserLoggingBase::TestAnalyserLoggingBase;

    void test_hybrid_diagnostics() {
        RealVariable x("x");
        DiscreteLocation q;
        HybridAutomaton system;
        system.new_mode(q, {dot(x) = -x});
        GeneralHybridEvolver evolver(system);
        LoggingAnalyser<HybridAutomatonInterface> analyser(evolver);
        check(analyser, HybridBoundedConstraintSet(q, {0.25_dec <= x <= 0.5_dec}),
              HybridBoundedConstraintSet(q, {-4 <= x <= 4}), HybridTime(Real(1 / 16_q), Integer(1)));
    }
};

int main(int argc, char** argv) {
    Logger::instance().redirect_to_file("test_analyser_logging_hybrid.log");
    Logger::instance().configuration().set_verbosity(6u);
    ARIADNE_TESTING_SUITE("test_analyser_logging_hybrid");
    ARIADNE_TESTING_ADD_TEST(TestAnalyserLoggingHybrid, test_hybrid_diagnostics);
    return ARIADNE_TESTING_RUN_TESTS_WITH_ARGS(argc, argv);
}


/***************************************************************************
 *            tests/test_generic_hybrid_grid_adapters.cpp
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

/** @file tests/test_generic_hybrid_grid_adapters.cpp
 *  @brief Check grid materialization without legacy hybrid grid or scaling headers.
 */

#include "reasoning/hybrid_grid_value_adapters.hpp"

#ifdef ARIADNE_HYBRID_GRID_HPP
#error Generic grid adapters must not import the legacy HybridGrid header
#endif
#ifdef ARIADNE_HYBRID_SCALINGS_HPP
#error Generic grid adapters must not import the legacy HybridScalings header
#endif

#include "logging/test_runner.hpp"
#include "ariadne-testing.hpp"

using namespace Ariadne;

class TestGenericHybridGridAdapters : public Ariadne::Testing::ContextFixture {
  public:
    using ContextFixture::ContextFixture;

    Void test_lazy_scalings_and_ordered_grids() {
        const DiscreteLocation first(1), second(2), absent(3);
        const RealVariable x("x"), y("y");
        MonolithicHybridSpace spaces;
        spaces.new_location(first, RealSpace({x}));
        spaces.new_location(second, RealSpace({y, x}));
        const HybridSpace space(spaces);

        unsigned materializations = 0u;
        Hybrid<Scalings> scalings(space,
            [&materializations, x, y](const DiscreteLocation& location,
                                     const RealSpace&) {
                ++materializations;
                Map<RealVariable, ExactDouble> entries;
                entries.insert(x, location == DiscreteLocation(1) ? 2.0_x : 5.0_x);
                entries.insert(y, 3.0_x);
                return Scalings(1.0_x, entries);
            });

        Hybrid<Grid> grids = make_hybrid_grids(space, std::move(scalings));
        ARIADNE_TESTING_PROVE(grids.has_location(first));
        ARIADNE_TESTING_PROVE(grids.has_location(second));
        ARIADNE_TESTING_PROVE(not grids.has_location(absent));
        ARIADNE_TESTING_PROVE(not grids.has_value(first));
        ARIADNE_TESTING_PROVE(not grids.has_value(second));
        ARIADNE_TESTING_EXPECT_EQUAL(materializations, 0u);

        const Grid& one = grids[first];
        ARIADNE_TESTING_EXPECT_EQUAL(materializations, 1u);
        ARIADNE_TESTING_EXPECT_EQUAL(one.dimension(), 1u);
        ARIADNE_TESTING_EXPECT_EQUAL(one.lengths()[0].get_d(), 2.0);
        ARIADNE_TESTING_PROVE(not grids.has_value(second));
        ARIADNE_TESTING_EXPECT_EQUAL(grids.size(), 1u);

        const Grid& second_grid = grids[second];
        ARIADNE_TESTING_EXPECT_EQUAL(materializations, 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(second_grid.dimension(), 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(second_grid.lengths()[0].get_d(), 3.0);
        ARIADNE_TESTING_EXPECT_EQUAL(second_grid.lengths()[1].get_d(), 5.0);
        ARIADNE_TESTING_EXPECT_EQUAL(grids.size(), 2u);

        const Hybrid<Grid>& read_only = grids;
        ARIADNE_TESTING_PROVE(read_only[first] == one);
        ARIADNE_TESTING_PROVE(read_only[second] == second_grid);
        ARIADNE_TESTING_EXPECT_EQUAL(materializations, 2u);
        ARIADNE_TESTING_EXPECT_EQUAL(read_only.materialized_locations().size(), 2u);
        ARIADNE_TESTING_EXPECT_EXCEPTION(grids[absent]);
    }
};

int main(int argc, char** argv) {
    ARIADNE_TESTING_SUITE("test_generic_hybrid_grid_adapters");
    ARIADNE_TESTING_ADD_TEST(TestGenericHybridGridAdapters, test_lazy_scalings_and_ordered_grids);
    return Ariadne::Testing::run_logging_suite(ariadne_test_suite, argc, argv);
}

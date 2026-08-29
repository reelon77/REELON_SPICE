// M07 simulation controller tests (AI-authored; see docs/AI参与记录.md).
// The controller implementation in src/sim remains user-authored.
#include "parser/Circuit.h"
#include "sim/simulate.h"

#include <gtest/gtest.h>

#include <sstream>
#include <variant>

TEST(SimulationControllerOpTest, ParsedDividerReturnsTypedHandCalculatedOperatingPoint) {
    std::istringstream input(
        "V1 1 0 10\n"
        "R1 1 2 1k\n"
        "R2 2 0 4k\n"
        ".op\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    const SimulationResult result = simulate(circuit);

    ASSERT_TRUE(std::holds_alternative<OperatingPointResult>(result));
    const OperatingPointResult& op = std::get<OperatingPointResult>(result);
    ASSERT_EQ(op.x.size(), 3u);
    EXPECT_NEAR(op.x[0], 10.0, 1e-9);
    EXPECT_NEAR(op.x[1], 8.0, 1e-9);
    EXPECT_NEAR(op.x[2], -2e-3, 1e-9);
    EXPECT_GT(op.iterations, 0);
}

TEST(SimulationControllerOpTest, ParsedDiodePreservesNewtonPathAndIterationInfo) {
    std::istringstream input(
        "V1 1 0 5\n"
        "R1 1 2 1k\n"
        "D1 2 0\n"
        ".op\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    const SimulationResult result = simulate(circuit);

    ASSERT_TRUE(std::holds_alternative<OperatingPointResult>(result));
    const OperatingPointResult& op = std::get<OperatingPointResult>(result);
    ASSERT_EQ(op.x.size(), 3u);
    EXPECT_NEAR(op.x[0], 5.0, 1e-9);
    EXPECT_NEAR(op.x[1], 0.574191503, 1e-6);
    EXPECT_NEAR(op.x[2], -4.425808e-3, 1e-6);
    EXPECT_GT(op.iterations, 1);
}

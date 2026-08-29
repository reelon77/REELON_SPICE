// M07 simulation controller tests (AI-authored; see docs/AI参与记录.md).
// The controller implementation in src/sim remains user-authored.
#include "parser/Circuit.h"
#include "sim/simulate.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <sstream>
#include <stdexcept>
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

TEST(SimulationControllerTransientTest, ParsedRcReturnsZeroInitialPointAndHandCalculatedTrajectory) {
    std::istringstream input(
        "V1 in 0 2\n"
        "R1 in out 2\n"
        "C1 out 0 500m\n"
        ".tran 250m 500m\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    const SimulationResult result = simulate(circuit);

    ASSERT_TRUE(std::holds_alternative<TransientAnalysisResult>(result));
    const TransientAnalysisResult& tran =
        std::get<TransientAnalysisResult>(result);
    ASSERT_EQ(tran.trajectory.size(), 3u);

    EXPECT_DOUBLE_EQ(tran.trajectory[0].time, 0.0);
    ASSERT_EQ(tran.trajectory[0].x.size(), 3u);
    EXPECT_DOUBLE_EQ(tran.trajectory[0].x[0], 0.0);
    EXPECT_DOUBLE_EQ(tran.trajectory[0].x[1], 0.0);
    EXPECT_DOUBLE_EQ(tran.trajectory[0].x[2], 0.0);

    EXPECT_NEAR(tran.trajectory[1].time, 0.25, 1e-12);
    EXPECT_NEAR(tran.trajectory[1].x[0], 2.0, 1e-12);
    EXPECT_NEAR(tran.trajectory[1].x[1], 0.4, 1e-12);
    EXPECT_NEAR(tran.trajectory[1].x[2], -0.8, 1e-12);

    EXPECT_NEAR(tran.trajectory[2].time, 0.5, 1e-12);
    EXPECT_NEAR(tran.trajectory[2].x[0], 2.0, 1e-12);
    EXPECT_NEAR(tran.trajectory[2].x[1], 0.72, 1e-12);
    EXPECT_NEAR(tran.trajectory[2].x[2], -0.64, 1e-12);
}

TEST(SimulationControllerTransientTest, ParsedRlPreservesSharedBranchOrderAndHandCalculatedTrajectory) {
    std::istringstream input(
        "L1 out 0 500m\n"
        "V1 in 0 2\n"
        "R1 in out 2\n"
        ".tran 250m 500m\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    const SimulationResult result = simulate(circuit);

    ASSERT_TRUE(std::holds_alternative<TransientAnalysisResult>(result));
    const TransientAnalysisResult& tran =
        std::get<TransientAnalysisResult>(result);
    ASSERT_EQ(tran.trajectory.size(), 3u);

    EXPECT_DOUBLE_EQ(tran.trajectory[0].time, 0.0);
    ASSERT_EQ(tran.trajectory[0].x.size(), 4u);
    EXPECT_DOUBLE_EQ(tran.trajectory[0].x[0], 0.0);
    EXPECT_DOUBLE_EQ(tran.trajectory[0].x[1], 0.0);
    EXPECT_DOUBLE_EQ(tran.trajectory[0].x[2], 0.0);
    EXPECT_DOUBLE_EQ(tran.trajectory[0].x[3], 0.0);

    EXPECT_NEAR(tran.trajectory[1].time, 0.25, 1e-12);
    EXPECT_NEAR(tran.trajectory[1].x[0], 1.0, 1e-12);
    EXPECT_NEAR(tran.trajectory[1].x[1], 2.0, 1e-12);
    EXPECT_NEAR(tran.trajectory[1].x[2], 0.5, 1e-12);
    EXPECT_NEAR(tran.trajectory[1].x[3], -0.5, 1e-12);

    EXPECT_NEAR(tran.trajectory[2].time, 0.5, 1e-12);
    EXPECT_NEAR(tran.trajectory[2].x[0], 0.5, 1e-12);
    EXPECT_NEAR(tran.trajectory[2].x[1], 2.0, 1e-12);
    EXPECT_NEAR(tran.trajectory[2].x[2], 0.75, 1e-12);
    EXPECT_NEAR(tran.trajectory[2].x[3], -0.75, 1e-12);
}

TEST(SimulationControllerErrorTest, RejectsCircuitWithoutAnalysisDirective) {
    std::istringstream input(
        "V1 1 0 1\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    ASSERT_EQ(circuit.analysis_type, AnalysisType::None);

    EXPECT_THROW(simulate(circuit), std::invalid_argument);
}

TEST(SimulationControllerErrorTest, PropagatesSingularOperatingPointFailure) {
    std::istringstream input(
        "R1 1 2 1k\n"
        ".op\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);

    EXPECT_THROW(simulate(circuit), std::runtime_error);
}

TEST(SimulationControllerErrorTest, PropagatesInvalidTransientParameters) {
    std::istringstream input(
        "V1 in 0 1\n"
        "R1 in out 1\n"
        "C1 out 0 1\n"
        ".tran 100m 200m\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    circuit.t_step = 0.0;

    EXPECT_THROW(simulate(circuit), std::invalid_argument);
}

TEST(SimulationControllerLifetimeTest, RepeatedTransientCallsDoNotShareWorkspaceOrHistory) {
    std::istringstream input(
        "V1 in 0 2\n"
        "R1 in out 2\n"
        "C1 out 0 500m\n"
        ".tran 250m 500m\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    const SimulationResult first_result = simulate(circuit);
    const SimulationResult second_result = simulate(circuit);

    ASSERT_TRUE(std::holds_alternative<TransientAnalysisResult>(first_result));
    ASSERT_TRUE(std::holds_alternative<TransientAnalysisResult>(second_result));
    const auto& first = std::get<TransientAnalysisResult>(first_result).trajectory;
    const auto& second = std::get<TransientAnalysisResult>(second_result).trajectory;

    ASSERT_EQ(first.size(), second.size());
    for (std::size_t point = 0; point < first.size(); ++point) {
        EXPECT_DOUBLE_EQ(first[point].time, second[point].time);
        EXPECT_EQ(first[point].x, second[point].x);
    }
}

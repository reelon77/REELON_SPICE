// M07 simulation controller tests (AI-authored; see docs/AI参与记录.md).
// The controller implementation in src/sim remains user-authored.
#include "parser/Circuit.h"
#include "sim/simulate.h"
#include "devices/IndependentSource.h"
#include "devices/Resistor.h"
#include "devices/VoltageSource.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <variant>

namespace {
const DcSweepAnalysisResult& as_dc(const SimulationResult& result) {
    EXPECT_TRUE(std::holds_alternative<DcSweepAnalysisResult>(result));
    return std::get<DcSweepAnalysisResult>(result);
}

class InitialGuessProbe final : public Device {
public:
    void stamp(MnaSystem&) const override {}

    void stamp(MnaSystem&, const std::vector<double>& x) const override {
        guesses.push_back(x);
    }

    mutable std::vector<std::vector<double>> guesses;
};
} // namespace

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
    ASSERT_TRUE(std::holds_alternative<std::monostate>(circuit.analysis));

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
    circuit.analysis = TransientAnalysis{0.0, 0.2};

    EXPECT_THROW(simulate(circuit), std::invalid_argument);
}

TEST(SimulationControllerValidationTest, RejectsDeviceIdentityMismatch) {
    std::istringstream input(
        "V1 1 0 1\n"
        ".op\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    circuit.device_names.clear();

    EXPECT_THROW(simulate(circuit), std::invalid_argument);
}

TEST(SimulationControllerValidationTest, RejectsInvalidHandBuiltDeviceIdentity) {
    {
        std::istringstream input(
            "V1 1 0 1\n"
            "R1 1 0 1k\n"
            ".op\n"
            ".end\n");
        Circuit circuit = parse_circuit(input);
        circuit.devices[0].reset();
        EXPECT_THROW(simulate(circuit), std::invalid_argument);
    }
    {
        std::istringstream input(
            "V1 1 0 1\n"
            "R1 1 0 1k\n"
            ".op\n"
            ".end\n");
        Circuit circuit = parse_circuit(input);
        circuit.device_names[1] = circuit.device_names[0];
        EXPECT_THROW(simulate(circuit), std::invalid_argument);
    }
    {
        std::istringstream input(
            "V1 1 0 1\n"
            ".op\n"
            ".end\n");
        Circuit circuit = parse_circuit(input);
        circuit.device_names[0] = "V1";
        EXPECT_THROW(simulate(circuit), std::invalid_argument);
    }
    {
        std::istringstream input(
            "V1 a 0 0\n"
            "V2 b 0 0\n"
            ".dc v1 0 1 1 v2 0 1 1\n"
            ".end\n");
        Circuit circuit = parse_circuit(input);
        auto& analysis = std::get<DcSweepAnalysis>(circuit.analysis);
        analysis.secondary->source_name = analysis.primary.source_name;
        EXPECT_THROW(simulate(circuit), std::invalid_argument);
    }
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

TEST(SimulationControllerDcTest, SingleVoltageSourceDividerMatchesHandOracle) {
    std::istringstream input(
        "V1 in 0 0\n"
        "R1 in out 1k\n"
        "R2 out 0 1k\n"
        ".dc V1 0 2 1\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    const SimulationResult result = simulate(circuit);
    const DcSweepAnalysisResult& dc = as_dc(result);

    ASSERT_EQ(dc.source_names, (std::vector<std::string>{"v1"}));
    ASSERT_EQ(dc.points.size(), 3u);
    const std::vector<std::vector<double>> expected_x{
        {0.0, 0.0, 0.0},
        {1.0, 0.5, -0.5e-3},
        {2.0, 1.0, -1.0e-3},
    };
    for (std::size_t point = 0; point < dc.points.size(); ++point) {
        ASSERT_EQ(dc.points[point].source_values.size(), 1u);
        EXPECT_DOUBLE_EQ(
            dc.points[point].source_values[0],
            static_cast<double>(point));
        ASSERT_EQ(dc.points[point].x.size(), expected_x[point].size());
        for (std::size_t entry = 0; entry < expected_x[point].size(); ++entry) {
            EXPECT_NEAR(
                dc.points[point].x[entry],
                expected_x[point][entry],
                1e-12);
        }
        EXPECT_GT(dc.points[point].newton_iterations, 0);
    }
}

TEST(SimulationControllerDcTest, DescendingCurrentSourceMatchesKclHandOracle) {
    std::istringstream input(
        "I1 out 0 0\n"
        "R1 out 0 1k\n"
        ".dc i1 2m 0 -1m\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    const SimulationResult result = simulate(circuit);
    const DcSweepAnalysisResult& dc = as_dc(result);

    ASSERT_EQ(dc.points.size(), 3u);
    const std::vector<double> source_values{2e-3, 1e-3, 0.0};
    const std::vector<double> node_voltages{-2.0, -1.0, 0.0};
    for (std::size_t point = 0; point < dc.points.size(); ++point) {
        EXPECT_NEAR(dc.points[point].source_values[0], source_values[point], 1e-15);
        ASSERT_EQ(dc.points[point].x.size(), 1u);
        EXPECT_NEAR(dc.points[point].x[0], node_voltages[point], 1e-12)
            << "KCL: V/1k + I1 = 0";
    }
}

TEST(SimulationControllerDcTest, DoubleSweepUsesSecondSourceAsOuterLoop) {
    std::istringstream input(
        "V1 a 0 0\n"
        "R1 a 0 1k\n"
        "V2 b 0 0\n"
        "R2 b 0 2k\n"
        ".dc v1 1 2 1 v2 10 20 10\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    const SimulationResult result = simulate(circuit);
    const DcSweepAnalysisResult& dc = as_dc(result);

    EXPECT_EQ(dc.source_names, (std::vector<std::string>{"v1", "v2"}));
    ASSERT_EQ(dc.points.size(), 4u);
    const std::vector<std::vector<double>> expected_values{
        {1.0, 10.0},
        {2.0, 10.0},
        {1.0, 20.0},
        {2.0, 20.0},
    };
    for (std::size_t point = 0; point < dc.points.size(); ++point) {
        EXPECT_EQ(dc.points[point].source_values, expected_values[point]);
        ASSERT_EQ(dc.points[point].x.size(), 4u);
        EXPECT_NEAR(dc.points[point].x[0], expected_values[point][0], 1e-12);
        EXPECT_NEAR(dc.points[point].x[1], expected_values[point][1], 1e-12);
        EXPECT_NEAR(dc.points[point].x[2], -expected_values[point][0] / 1e3, 1e-12);
        EXPECT_NEAR(dc.points[point].x[3], -expected_values[point][1] / 2e3, 1e-12);
    }
}

TEST(SimulationControllerDcTest, CarriesPreviousSuccessfulPointAsInitialGuess) {
    Circuit circuit;
    circuit.nodes = 2;
    circuit.num_branch_unknowns = 1;
    circuit.node_names = {"0", "n"};
    circuit.branch_names = {"v1"};
    circuit.device_names = {"v1", "r1", "probe"};
    circuit.devices.push_back(
        std::make_unique<VoltageSource>(0.0, 1, 0, 0));
    circuit.devices.push_back(std::make_unique<Resistor>(1e3, 1, 0));
    auto probe = std::make_unique<InitialGuessProbe>();
    InitialGuessProbe* probe_view = probe.get();
    circuit.devices.push_back(std::move(probe));
    circuit.analysis = DcSweepAnalysis{
        DcSweepAxis{"v1", 0u, 1.0, 2.0, 1.0},
        std::nullopt,
    };

    const SimulationResult result = simulate(circuit);
    const DcSweepAnalysisResult& dc = as_dc(result);

    ASSERT_EQ(dc.points.size(), 2u);
    const std::size_t second_point_first_iteration =
        static_cast<std::size_t>(dc.points[0].newton_iterations);
    ASSERT_LT(second_point_first_iteration, probe_view->guesses.size());
    EXPECT_EQ(
        probe_view->guesses[second_point_first_iteration],
        dc.points[0].x);
    ASSERT_EQ(probe_view->guesses[0].size(), 2u);
    EXPECT_EQ(probe_view->guesses[0], (std::vector<double>{0.0, 0.0}));
}

TEST(SimulationControllerDcTest, NonlinearSweepMatchesReferenceAndKclResidual) {
    std::istringstream input(
        "V1 in 0 0\n"
        "R1 in out 1k\n"
        "D1 out 0\n"
        ".dc v1 0 5 5\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    const SimulationResult result = simulate(circuit);
    const DcSweepAnalysisResult& dc = as_dc(result);

    ASSERT_EQ(dc.points.size(), 2u);
    EXPECT_NEAR(dc.points[1].x[1], 0.574191503, 1e-6);
    for (const DcSweepPoint& point : dc.points) {
        const double source_voltage = point.source_values[0];
        const double diode_voltage = point.x[1];
        const double resistor_current =
            (source_voltage - diode_voltage) / 1e3;
        const double diode_current =
            1e-12 * std::expm1(diode_voltage / 0.025852);
        EXPECT_NEAR(resistor_current - diode_current, 0.0, 1e-9);
    }
}

TEST(SimulationControllerDcTest, RepeatedCallsDoNotMutateOriginalSource) {
    std::istringstream input(
        "V1 n 0 7\n"
        "R1 n 0 1k\n"
        ".dc v1 0 2 1\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    const auto* original_source = dynamic_cast<const IndependentSource*>(
        circuit.devices[0].get());
    ASSERT_NE(original_source, nullptr);
    const SimulationResult first = simulate(circuit);
    const SimulationResult second = simulate(circuit);

    EXPECT_DOUBLE_EQ(original_source->dc_value(), 7.0);
    const auto& first_points = as_dc(first).points;
    const auto& second_points = as_dc(second).points;
    ASSERT_EQ(first_points.size(), second_points.size());
    for (std::size_t point = 0; point < first_points.size(); ++point) {
        EXPECT_EQ(first_points[point].source_values, second_points[point].source_values);
        EXPECT_EQ(first_points[point].x, second_points[point].x);
        EXPECT_EQ(
            first_points[point].newton_iterations,
            second_points[point].newton_iterations);
    }
}

TEST(SimulationControllerDcErrorTest, ReportsPointAndValuesOnSolverFailure) {
    std::istringstream input(
        "V1 driven 0 0\n"
        "RFloat a b 1k\n"
        ".dc v1 0 1 1\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    try {
        (void)simulate(circuit);
        FAIL() << "floating DC sweep must fail";
    } catch (const std::runtime_error& error) {
        const std::string message = error.what();
        EXPECT_NE(message.find("DC sweep point 1"), std::string::npos);
        EXPECT_NE(message.find("v1=0"), std::string::npos);
    }
}

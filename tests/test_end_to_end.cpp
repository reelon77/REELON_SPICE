// 端到端闸门测试（AI 代写，见 docs/AI参与记录.md）
// 第 1 周闸门定义：分压电路直流工作点，v2 = 8V。
// 链路：手工网表 → 器件 stamp 进 MnaSystem → LU 分解 → 求解 → 断言三个量。
#include "core/LU.h"
#include "devices/Capacitor.h"
#include "devices/Inductor.h"
#include "devices/Resistor.h"
#include "devices/VoltageSource.h"
#include "mna/MnaSystem.h"
#include "parser/Circuit.h"
#include "solver/newton.h"
#include "solver/transient.h"
#include <gtest/gtest.h>
#include <sstream>
#include <variant>
#include <vector>

namespace {

std::vector<Device*> borrow_devices(const Circuit& circuit) {
    std::vector<Device*> devices;
    devices.reserve(circuit.devices.size());
    for (const auto& device : circuit.devices) {
        devices.push_back(device.get());
    }
    return devices;
}

void expect_trajectories_near(
    const std::vector<TransientPoint>& actual,
    const std::vector<TransientPoint>& expected,
    double tolerance) {
    ASSERT_EQ(actual.size(), expected.size());
    for (std::size_t point = 0; point < actual.size(); ++point) {
        EXPECT_NEAR(actual[point].time, expected[point].time, tolerance);
        ASSERT_EQ(actual[point].x.size(), expected[point].x.size());
        for (std::size_t entry = 0; entry < actual[point].x.size(); ++entry) {
            EXPECT_NEAR(actual[point].x[entry], expected[point].x[entry], tolerance)
                << "trajectory point " << point << ", x entry " << entry;
        }
    }
}

} // namespace

// 电路：V1 = 10V（节点1→地），R1 = 1kΩ（1↔2），R2 = 4kΩ（2↔地）
// 手推：v1 = 10（源焊死）
//       v2 = 10·R2/(R1+R2) = 10·4k/5k = 8（分压）
//       i  = −10/(R1+R2) = −2mA（源供出 2mA，按"KCL 行 +i"约定取负）
TEST(EndToEndGateTest, VoltageDividerSolvesTo8V) {
    MnaSystem sys(3, 1);  // 节点 0(地)、1、2 + 1 个电压源 → dim = 3

    VoltageSource v1(10.0, 1, 0, 0);
    Resistor r1(1000.0, 1, 2);
    Resistor r2(4000.0, 2, 0);
    v1.stamp(sys);
    r1.stamp(sys);
    r2.stamp(sys);

    LUResult lu = lu_decomposition(sys.get_A());
    std::vector<double> x = lu_solve(lu, sys.get_b());

    ASSERT_EQ(static_cast<int>(x.size()), 3);
    EXPECT_NEAR(x[0], 10.0, 1e-9) << "v1 必须等于源电压";
    EXPECT_NEAR(x[1], 8.0, 1e-9) << "v2 = Vs·R2/(R1+R2)，闸门核心断言";
    EXPECT_NEAR(x[2], -2e-3, 1e-9) << "i_V1 = -Vs/(R1+R2)";
}

// 解析器第 4 档:真实网表文本贯通 parser → MNA → solver。
// 本组测试由 AI 代写，parser 与全部求解器/器件实现均由用户亲手编写。

TEST(ParserEndToEndTest, VoltageDividerNetlistSolvesTo8V) {
    std::istringstream input(
        "V1 1 0 10\n"
        "R1 1 2 1k\n"
        "R2 2 0 4k\n"
        ".op\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    ASSERT_TRUE(std::holds_alternative<OperatingPointAnalysis>(circuit.analysis));
    ASSERT_EQ(circuit.nodes, 3);
    ASSERT_EQ(circuit.num_branch_unknowns, 1);

    MnaSystem sys(circuit.nodes, circuit.num_branch_unknowns);
    for (Device* device : borrow_devices(circuit)) {
        device->stamp(sys);
    }
    const LUResult lu = lu_decomposition(sys.get_A());
    const std::vector<double> x = lu_solve(lu, sys.get_b());

    ASSERT_EQ(x.size(), 3u);
    EXPECT_NEAR(x[0], 10.0, 1e-9);
    EXPECT_NEAR(x[1], 8.0, 1e-9);
    EXPECT_NEAR(x[2], -2e-3, 1e-9);
}

TEST(ParserEndToEndTest, DiodeNetlistConvergesToExpectedOperatingPoint) {
    std::istringstream input(
        "V1 1 0 5\n"
        "R1 1 2 1k\n"
        "D1 2 0\n"
        ".op\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    ASSERT_TRUE(std::holds_alternative<OperatingPointAnalysis>(circuit.analysis));
    ASSERT_EQ(circuit.nodes, 3);
    ASSERT_EQ(circuit.num_branch_unknowns, 1);

    MnaSystem sys(circuit.nodes, circuit.num_branch_unknowns);
    const NewtonResult result = newton_solve(borrow_devices(circuit), sys);

    ASSERT_EQ(result.x.size(), 3u);
    EXPECT_NEAR(result.x[0], 5.0, 1e-9);
    EXPECT_NEAR(result.x[1], 0.574191503, 1e-6);
    EXPECT_NEAR(result.x[2], -4.425808e-3, 1e-6);
}

// parser -> 瞬态 RC 闭环。独立手算：Vs=2、R=2、C=0.5、dt=0.25，
// beta=RC/(RC+dt)=0.8，所以 vC1=0.4、vC2=0.72；
// iV=-(Vs-vC)/R，前两步分别为 -0.8、-0.64。
TEST(ParserTransientEndToEndTest, RcNetlistMatchesManualConstructionAndHandOracle) {
    std::istringstream input(
        "V1 in 0 2\n"
        "R1 in out 2\n"
        "C1 out 0 500m\n"
        ".tran 250m 500m\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    ASSERT_TRUE(std::holds_alternative<TransientAnalysis>(circuit.analysis));
    const auto& transient = std::get<TransientAnalysis>(circuit.analysis);
    ASSERT_EQ(circuit.nodes, 3);
    ASSERT_EQ(circuit.num_branch_unknowns, 1);

    MnaSystem parsed_sys(circuit.nodes, circuit.num_branch_unknowns);
    const std::vector<double> parsed_initial_x(
        static_cast<std::size_t>(parsed_sys.dim()), 0.0);
    const std::vector<TransientPoint> parsed = transient_solve(
        borrow_devices(circuit),
        parsed_sys,
        transient.t_step,
        transient.t_stop,
        parsed_initial_x);

    VoltageSource manual_source(2.0, 1, 0, 0);
    Resistor manual_resistor(2.0, 1, 2);
    Capacitor manual_capacitor(0.5, 2, 0);
    std::vector<Device*> manual_devices{
        &manual_source,
        &manual_resistor,
        &manual_capacitor,
    };
    MnaSystem manual_sys(3, 1);
    const std::vector<double> manual_initial_x{0.0, 0.0, 0.0};
    const std::vector<TransientPoint> manual = transient_solve(
        manual_devices, manual_sys, 0.25, 0.5, manual_initial_x);

    expect_trajectories_near(parsed, manual, 1e-12);
    ASSERT_EQ(parsed.size(), 3u);
    EXPECT_EQ(parsed[0].x, parsed_initial_x);
    EXPECT_NEAR(parsed[1].time, 0.25, 1e-12);
    EXPECT_NEAR(parsed[1].x[0], 2.0, 1e-12);
    EXPECT_NEAR(parsed[1].x[1], 0.4, 1e-12);
    EXPECT_NEAR(parsed[1].x[2], -0.8, 1e-12);
    EXPECT_NEAR(parsed[2].time, 0.5, 1e-12);
    EXPECT_NEAR(parsed[2].x[0], 2.0, 1e-12);
    EXPECT_NEAR(parsed[2].x[1], 0.72, 1e-12);
    EXPECT_NEAR(parsed[2].x[2], -0.64, 1e-12);
}

// L 故意写在 V 前，锁定全局支路按网表顺序编号：
// 节点 [out,in]，支路 [iL,iV]。独立手算 beta=L/(L+Rdt)=0.5、Iinf=1，
// 因此 iL1=0.5、iL2=0.75，vout=Vs-R*iL 分别为 1、0.5，iV=-iL。
TEST(ParserTransientEndToEndTest, RlNetlistSharesBranchPoolAndMatchesHandOracle) {
    std::istringstream input(
        "L1 out 0 500m\n"
        "V1 in 0 2\n"
        "R1 in out 2\n"
        ".tran 250m 500m\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);
    ASSERT_TRUE(std::holds_alternative<TransientAnalysis>(circuit.analysis));
    const auto& transient = std::get<TransientAnalysis>(circuit.analysis);
    ASSERT_EQ(circuit.nodes, 3);
    ASSERT_EQ(circuit.num_branch_unknowns, 2);

    MnaSystem parsed_sys(circuit.nodes, circuit.num_branch_unknowns);
    const std::vector<double> parsed_initial_x(
        static_cast<std::size_t>(parsed_sys.dim()), 0.0);
    const std::vector<TransientPoint> parsed = transient_solve(
        borrow_devices(circuit),
        parsed_sys,
        transient.t_step,
        transient.t_stop,
        parsed_initial_x);

    Inductor manual_inductor(0.5, 1, 0, 0);
    VoltageSource manual_source(2.0, 2, 0, 1);
    Resistor manual_resistor(2.0, 2, 1);
    std::vector<Device*> manual_devices{
        &manual_inductor,
        &manual_source,
        &manual_resistor,
    };
    MnaSystem manual_sys(3, 2);
    const std::vector<double> manual_initial_x{0.0, 0.0, 0.0, 0.0};
    const std::vector<TransientPoint> manual = transient_solve(
        manual_devices, manual_sys, 0.25, 0.5, manual_initial_x);

    expect_trajectories_near(parsed, manual, 1e-12);
    ASSERT_EQ(parsed.size(), 3u);
    EXPECT_EQ(parsed[0].x, parsed_initial_x);
    EXPECT_NEAR(parsed[1].time, 0.25, 1e-12);
    EXPECT_NEAR(parsed[1].x[0], 1.0, 1e-12);
    EXPECT_NEAR(parsed[1].x[1], 2.0, 1e-12);
    EXPECT_NEAR(parsed[1].x[2], 0.5, 1e-12);
    EXPECT_NEAR(parsed[1].x[3], -0.5, 1e-12);
    EXPECT_NEAR(parsed[2].time, 0.5, 1e-12);
    EXPECT_NEAR(parsed[2].x[0], 0.5, 1e-12);
    EXPECT_NEAR(parsed[2].x[1], 2.0, 1e-12);
    EXPECT_NEAR(parsed[2].x[2], 0.75, 1e-12);
    EXPECT_NEAR(parsed[2].x[3], -0.75, 1e-12);
}

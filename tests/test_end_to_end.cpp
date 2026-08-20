// 端到端闸门测试（AI 代写，见 docs/AI参与记录.md）
// 第 1 周闸门定义：分压电路直流工作点，v2 = 8V。
// 链路：手工网表 → 器件 stamp 进 MnaSystem → LU 分解 → 求解 → 断言三个量。
#include "core/LU.h"
#include "devices/Resistor.h"
#include "devices/VoltageSource.h"
#include "mna/MnaSystem.h"
#include "parser/Circuit.h"
#include "solver/newton.h"
#include <gtest/gtest.h>
#include <sstream>
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
    ASSERT_EQ(circuit.analysis_type, AnalysisType::Op);
    ASSERT_EQ(circuit.nodes, 3);
    ASSERT_EQ(circuit.num_voltage_sources, 1);

    MnaSystem sys(circuit.nodes, circuit.num_voltage_sources);
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
    ASSERT_EQ(circuit.analysis_type, AnalysisType::Op);
    ASSERT_EQ(circuit.nodes, 3);
    ASSERT_EQ(circuit.num_voltage_sources, 1);

    MnaSystem sys(circuit.nodes, circuit.num_voltage_sources);
    const NewtonResult result = newton_solve(borrow_devices(circuit), sys);

    ASSERT_EQ(result.x.size(), 3u);
    EXPECT_NEAR(result.x[0], 5.0, 1e-9);
    EXPECT_NEAR(result.x[1], 0.574191503, 1e-6);
    EXPECT_NEAR(result.x[2], -4.425808e-3, 1e-6);
}

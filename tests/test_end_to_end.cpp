// 端到端闸门测试（AI 代写，见 docs/AI参与记录.md）
// 第 1 周闸门定义：分压电路直流工作点，v2 = 8V。
// 链路：手工网表 → 器件 stamp 进 MnaSystem → LU 分解 → 求解 → 断言三个量。
#include "core/LU.h"
#include "devices/Resistor.h"
#include "devices/VoltageSource.h"
#include "mna/MnaSystem.h"
#include <gtest/gtest.h>

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

// 牛顿求解器测试(AI 代写,见 docs/AI参与记录.md)
// 数值依据:Day4 交接单实算 —— 真解 v2 = 0.574191503 V,回路电流 4.425808 mA;
// 初值全 0 时约 178 次迭代(个位数差异正常,量级不对才是 bug)。
#include "solver/newton.h"
#include "devices/Resistor.h"
#include "devices/VoltageSource.h"
#include "devices/Diode.h"
#include <gtest/gtest.h>
#include <stdexcept>

// ============================================================
// 第 3 档 · 烟囱测试:纯线性电路走牛顿循环
// Day 3 闸门电路:V1=10V(1→地), R1=1k(1↔2), R2=4k(2↔地)
// 线性电路的切线就是它自己 ⇒ 第 1 轮已得真解,第 2 轮 diff=0 收敛。
// 这一档专门守"clear() + 全体重盖"没写错:若漏清零,第 2 轮矩阵翻倍,答案跑飞。
// ============================================================

TEST(NewtonSmokeTest, LinearDividerConvergesInstantlyToSameAnswer) {
    MnaSystem sys(3, 1);
    VoltageSource v1(10.0, 1, 0, 0);
    Resistor r1(1000.0, 1, 2);
    Resistor r2(4000.0, 2, 0);
    std::vector<Device*> devices{&v1, &r1, &r2};

    NewtonResult r = newton_solve(devices, sys, 1e-9, 300);

    EXPECT_LE(r.iterations, 2) << "线性电路应 1~2 轮收敛,更多轮 = 每轮组装出的方程不一致";
    ASSERT_EQ(static_cast<int>(r.x.size()), 3);
    EXPECT_NEAR(r.x[0], 10.0, 1e-9);
    EXPECT_NEAR(r.x[1], 8.0, 1e-9) << "答案必须仍是 Day 3 的 8V";
    EXPECT_NEAR(r.x[2], -2e-3, 1e-9);
}

// ============================================================
// 第 4 档 · 端到端:二极管电路(今天的闸门,08-05 里程碑)
// V1=5V(1→地), R=1k(1↔2), D(2→地,阳极接 2)
// ============================================================

TEST(NewtonEndToEndTest, DiodeCircuitConvergesToTrueSolution) {
    MnaSystem sys(3, 1);
    VoltageSource v1(5.0, 1, 0, 0);
    Resistor r(1000.0, 1, 2);
    Diode d(1e-12, 0.025852, 2, 0);
    std::vector<Device*> devices{&v1, &r, &d};

    NewtonResult result = newton_solve(devices, sys, 1e-9, 300);

    ASSERT_EQ(static_cast<int>(result.x.size()), 3);
    EXPECT_NEAR(result.x[0], 5.0, 1e-9) << "v1 被源钉死";
    EXPECT_NEAR(result.x[1], 0.574191503, 1e-6) << "二极管工作点,闸门核心断言";
    EXPECT_NEAR(result.x[2], -4.425808e-3, 1e-6) << "回路电流 (v2−5)/R";
    EXPECT_GE(result.iterations, 10) << "几轮就'收敛'多半是二极管没参与";
    EXPECT_LE(result.iterations, 300) << "量级失控";
}

// ============================================================
// 第 5 档 · 不收敛路径:保险丝必须响
// ============================================================

TEST(NewtonFailureTest, ThrowsWhenMaxIterTooSmall) {
    MnaSystem sys(3, 1);
    VoltageSource v1(5.0, 1, 0, 0);
    Resistor r(1000.0, 1, 2);
    Diode d(1e-12, 0.025852, 2, 0);
    std::vector<Device*> devices{&v1, &r, &d};

    EXPECT_THROW(newton_solve(devices, sys, 1e-9, 5), std::runtime_error);
}

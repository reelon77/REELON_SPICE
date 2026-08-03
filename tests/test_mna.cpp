// MNA 组装器测试（AI 代写，见 docs/AI参与记录.md）
// 覆盖：电阻 stamp 的落点/符号/对称性、共享节点电导累加（+= 卫兵）、
//       接地器件只触碰非地下标、输入校验。
#include "devices/Resistor.h"
#include "mna/MnaSystem.h"
#include <gtest/gtest.h>
#include <stdexcept>

// 单个电阻 (1↔2, 2Ω) 盖进 2 节点系统：4 个落点 ±0.5，G 子块对称，b 不被触碰
TEST(MnaResistorTest, SingleResistorStampsFourEntries) {
    MnaSystem sys(3, 0);  // 节点 0(地)、1、2；无电压源
    EXPECT_EQ(sys.dim(), 2);

    Resistor r(2.0, 1, 2);
    r.stamp(sys);

    const Matrix& A = sys.get_A();
    EXPECT_DOUBLE_EQ(A(0, 0), 0.5);
    EXPECT_DOUBLE_EQ(A(1, 1), 0.5);
    EXPECT_DOUBLE_EQ(A(0, 1), -0.5);
    EXPECT_DOUBLE_EQ(A(1, 0), -0.5);
    EXPECT_DOUBLE_EQ(A(0, 1), A(1, 0)) << "G 子块必须对称";

    for (double v : sys.get_b()) {
        EXPECT_DOUBLE_EQ(v, 0.0) << "电阻不应触碰 b";
    }
}

// += 卫兵：第二个电阻 (2↔地, 4Ω) 共享节点 2，
// 共享对角元必须是两个电导之和，而不是被后来者覆盖
TEST(MnaResistorTest, SharedNodeConductancesAccumulate) {
    MnaSystem sys(3, 0);
    Resistor r1(2.0, 1, 2);   // g = 0.5
    Resistor r2(4.0, 2, 0);   // g = 0.25，一端接地
    r1.stamp(sys);
    r2.stamp(sys);

    const Matrix& A = sys.get_A();
    EXPECT_DOUBLE_EQ(A(1, 1), 0.75) << "共享节点电导应累加：0.5 + 0.25";
    // r2 接地，除 A(1,1) 外其余位置必须保持 r1 盖章后的原值
    EXPECT_DOUBLE_EQ(A(0, 0), 0.5);
    EXPECT_DOUBLE_EQ(A(0, 1), -0.5);
    EXPECT_DOUBLE_EQ(A(1, 0), -0.5);
}

// 接地电阻 (1↔地)：只有非地端的对角元被写，其余全零
TEST(MnaResistorTest, GroundedResistorTouchesOnlyOwnDiagonal) {
    MnaSystem sys(2, 0);  // 节点 0(地)、1 → dim = 1
    EXPECT_EQ(sys.dim(), 1);

    Resistor r(4.0, 1, 0);
    r.stamp(sys);

    EXPECT_DOUBLE_EQ(sys.get_A()(0, 0), 0.25);
    EXPECT_DOUBLE_EQ(sys.get_b()[0], 0.0);
}

// 输入校验：零阻（无穷电导）与负阻都应在构造时被拒绝
TEST(MnaResistorTest, NonPositiveResistanceThrows) {
    EXPECT_THROW(Resistor(0.0, 1, 2), std::invalid_argument);
    EXPECT_THROW(Resistor(-100.0, 1, 2), std::invalid_argument);
}

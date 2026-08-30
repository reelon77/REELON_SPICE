// I/V 源 stamp 测试（AI 代写，见 docs/AI参与记录.md）
// 覆盖：CurrentSource 的落点/符号、接地端跳过、与电阻共存互不污染；
//       VoltageSource 的维度扩展、混合坐标 5 个落点与符号配对、接地端只剩 3 次写入。
#include "devices/CurrentSource.h"
#include "devices/IndependentSource.h"
#include "devices/Resistor.h"
#include "devices/VoltageSource.h"
#include "mna/MnaSystem.h"
#include <gtest/gtest.h>

// 电流源 2mA (1→2)：从节点 1 抽走、注入节点 2，b(a) -= I、b(b) += I，A 完全不被触碰
TEST(CurrentSourceTest, StampsTwoEntriesWithCorrectSigns) {
    MnaSystem sys(3, 0);  // 节点 0(地)、1、2
    CurrentSource is(2e-3, 1, 2);
    is.stamp(sys);

    EXPECT_DOUBLE_EQ(sys.get_b()[0], -2e-3) << "流出端 b(a) -= I";
    EXPECT_DOUBLE_EQ(sys.get_b()[1], 2e-3) << "注入端 b(b) += I";

    const Matrix& A = sys.get_A();
    for (int i = 0; i < A.rows(); ++i) {
        for (int j = 0; j < A.cols(); ++j) {
            EXPECT_DOUBLE_EQ(A(i, j), 0.0) << "电流源不应触碰 A(" << i << "," << j << ")";
        }
    }
}

// 接地端跳过：地 → 节点1 只写 b(b)；节点2 → 地 只写 b(a)，不越界不落地
TEST(CurrentSourceTest, GroundedTerminalIsSkipped) {
    MnaSystem fromGround(3, 0);
    CurrentSource i1(1.0, 0, 1);  // 地 → 节点1
    i1.stamp(fromGround);
    EXPECT_DOUBLE_EQ(fromGround.get_b()[0], 1.0);
    EXPECT_DOUBLE_EQ(fromGround.get_b()[1], 0.0);

    MnaSystem toGround(3, 0);
    CurrentSource i2(1.0, 2, 0);  // 节点2 → 地
    i2.stamp(toGround);
    EXPECT_DOUBLE_EQ(toGround.get_b()[0], 0.0);
    EXPECT_DOUBLE_EQ(toGround.get_b()[1], -1.0);
}

// 与电阻共存：电阻只写 A、电流源只写 b，先后 stamp 互不污染
TEST(CurrentSourceTest, CoexistsWithResistorWithoutPollutingA) {
    MnaSystem sys(3, 0);
    Resistor r(2.0, 1, 2);        // g = 0.5
    CurrentSource is(3e-3, 1, 2);
    r.stamp(sys);
    is.stamp(sys);

    const Matrix& A = sys.get_A();
    EXPECT_DOUBLE_EQ(A(0, 0), 0.5);
    EXPECT_DOUBLE_EQ(A(1, 1), 0.5);
    EXPECT_DOUBLE_EQ(A(0, 1), -0.5);
    EXPECT_DOUBLE_EQ(A(1, 0), -0.5);

    EXPECT_DOUBLE_EQ(sys.get_b()[0], -3e-3);
    EXPECT_DOUBLE_EQ(sys.get_b()[1], 3e-3);
}

// 电压源引入支路未知量：dim = 节点数-1 + 源数，支路下标排在节点块之后
TEST(VoltageSourceTest, DimensionIncludesBranchUnknown) {
    MnaSystem sys(3, 1);  // 节点 0(地)、1、2 + 1 个电压源
    EXPECT_EQ(sys.dim(), 3);
    EXPECT_EQ(sys.branch_index(0), 2);
}

// 悬空电压源 (1↔2, 5V)：混合坐标 5 个落点，KCL 行与约束行符号配对，其余位置不被触碰
TEST(VoltageSourceTest, StampsFiveEntriesInMixedCoordinates) {
    MnaSystem sys(3, 1);
    VoltageSource vs(5.0, 1, 2, 0);  // a=1 正端，b=2，0 号源
    vs.stamp(sys);

    const Matrix& A = sys.get_A();
    const int k = 2;  // branch_index(0)

    EXPECT_DOUBLE_EQ(A(0, k), 1.0) << "KCL 行：节点 a 方程 +i";
    EXPECT_DOUBLE_EQ(A(1, k), -1.0) << "KCL 行：节点 b 方程 -i";
    EXPECT_DOUBLE_EQ(A(k, 0), 1.0) << "约束行：+v_a，与 A(a,k) 同号（转置对称）";
    EXPECT_DOUBLE_EQ(A(k, 1), -1.0) << "约束行：-v_b，与 A(b,k) 同号";
    EXPECT_DOUBLE_EQ(sys.get_b()[k], 5.0) << "约束右端 b(k) = V";

    // 电压源不该碰的地方必须干净：G 子块、支路对角元、b 的节点行
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            EXPECT_DOUBLE_EQ(A(i, j), 0.0) << "G 子块不应被电压源触碰";
        }
        EXPECT_DOUBLE_EQ(sys.get_b()[i], 0.0) << "b 的节点行不应被触碰";
    }
    EXPECT_DOUBLE_EQ(A(k, k), 0.0) << "支路对角元应为零（理想源无内阻）";
}

// 负端接地 (1↔地, 10V)：地端两次写入整体消失，5 次剩 3 次，且不越界
TEST(VoltageSourceTest, GroundedNegativeTerminalLeavesThreeEntries) {
    MnaSystem sys(2, 1);  // 节点 0(地)、1 + 1 个电压源 → dim = 2
    VoltageSource vs(10.0, 1, 0, 0);
    vs.stamp(sys);

    const Matrix& A = sys.get_A();
    const int k = 1;  // branch_index(0)
    EXPECT_DOUBLE_EQ(A(0, k), 1.0);
    EXPECT_DOUBLE_EQ(A(k, 0), 1.0);
    EXPECT_DOUBLE_EQ(sys.get_b()[k], 10.0);
    EXPECT_DOUBLE_EQ(A(0, 0), 0.0);
    EXPECT_DOUBLE_EQ(A(k, k), 0.0);
    EXPECT_DOUBLE_EQ(sys.get_b()[0], 0.0);
}

TEST(IndependentSourceTest, VoltageCloneChangesOnlyDcValueAndPreservesOriginal) {
    VoltageSource original(5.0, 1, 0, 0);
    IndependentSource& source = original;
    std::unique_ptr<IndependentSource> clone =
        source.clone_with_dc_value(9.0);

    EXPECT_DOUBLE_EQ(source.dc_value(), 5.0);
    EXPECT_DOUBLE_EQ(clone->dc_value(), 9.0);
    EXPECT_NE(dynamic_cast<VoltageSource*>(clone.get()), nullptr);

    MnaSystem original_before(2, 1);
    MnaSystem overridden(2, 1);
    MnaSystem original_after(2, 1);
    source.stamp(original_before);
    clone->stamp(overridden);
    source.stamp(original_after);

    for (int row = 0; row < original_before.dim(); ++row) {
        for (int column = 0; column < original_before.dim(); ++column) {
            EXPECT_DOUBLE_EQ(
                original_before.get_A()(row, column),
                overridden.get_A()(row, column));
            EXPECT_DOUBLE_EQ(
                original_before.get_A()(row, column),
                original_after.get_A()(row, column));
        }
    }
    EXPECT_DOUBLE_EQ(original_before.get_b()[1], 5.0);
    EXPECT_DOUBLE_EQ(overridden.get_b()[1], 9.0);
    EXPECT_DOUBLE_EQ(original_after.get_b()[1], 5.0);
}

TEST(IndependentSourceTest, CurrentCloneChangesOnlyDcValueAndPreservesOriginal) {
    CurrentSource original(2e-3, 1, 0);
    IndependentSource& source = original;
    std::unique_ptr<IndependentSource> clone =
        source.clone_with_dc_value(-3e-3);

    EXPECT_DOUBLE_EQ(source.dc_value(), 2e-3);
    EXPECT_DOUBLE_EQ(clone->dc_value(), -3e-3);
    EXPECT_NE(dynamic_cast<CurrentSource*>(clone.get()), nullptr);

    MnaSystem original_before(2, 0);
    MnaSystem overridden(2, 0);
    MnaSystem original_after(2, 0);
    source.stamp(original_before);
    clone->stamp(overridden);
    source.stamp(original_after);

    EXPECT_DOUBLE_EQ(original_before.get_b()[0], -2e-3);
    EXPECT_DOUBLE_EQ(overridden.get_b()[0], 3e-3);
    EXPECT_DOUBLE_EQ(original_after.get_b()[0], -2e-3);
}

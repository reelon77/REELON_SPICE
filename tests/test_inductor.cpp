// 电感后向欧拉 MNA 支路未知量测试（AI 代写，见 docs/AI参与记录.md）
// 独立 oracle：v_p-v_q-(L/dt)i_n=-(L/dt)i_prev。
// 电感电流未知量的正方向为 p->q，V/L 共用全局支路编号空间。
#include "devices/Inductor.h"
#include "devices/Resistor.h"
#include "devices/VoltageSource.h"
#include "mna/MnaSystem.h"
#include "solver/transient.h"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>
#include <vector>

namespace {

void expect_system_equals(
    const MnaSystem& sys,
    const std::vector<std::vector<double>>& expected_A,
    const std::vector<double>& expected_b) {
    ASSERT_EQ(sys.get_A().rows(), static_cast<int>(expected_A.size()));
    ASSERT_EQ(sys.get_A().cols(), static_cast<int>(expected_A.size()));
    ASSERT_EQ(sys.get_b().size(), expected_b.size());

    for (int row = 0; row < sys.get_A().rows(); ++row) {
        ASSERT_EQ(expected_A[row].size(), expected_A.size());
        for (int col = 0; col < sys.get_A().cols(); ++col) {
            EXPECT_DOUBLE_EQ(sys.get_A()(row, col), expected_A[row][col])
                << "matrix entry (" << row << ", " << col << ")";
        }
        EXPECT_DOUBLE_EQ(sys.get_b()[row], expected_b[row])
            << "rhs entry " << row;
    }
}

} // namespace

// 本任务只定义正电感；零或负值必须在构造期拒绝。
TEST(InductorConstructionTest, RejectsNonPositiveInductance) {
    EXPECT_THROW((Inductor(0.0, 1, 0, 0)), std::invalid_argument);
    EXPECT_THROW((Inductor(-2.0, 1, 0, 0)), std::invalid_argument);
}

// DC 稳态下 di/dt=0，所以 v_p-v_q=0。支路未知量 k=2 时，
// 只留下节点 KCL 列与约束行的四个耦合项，A(k,k) 和 b(k) 都为零。
TEST(InductorDcTest, StampsIdealShortWithBranchUnknown) {
    MnaSystem sys(3, 1);
    Inductor inductor(2.0, 1, 2, 0);

    inductor.stamp(sys);

    expect_system_equals(
        sys,
        {
            {0.0, 0.0, 1.0},
            {0.0, 0.0, -1.0},
            {1.0, -1.0, 0.0},
        },
        {0.0, 0.0, 0.0});
}

// 手算 oracle：L=2H、dt=0.5s、i_prev=3A，所以 L/dt=4ohm，
// 支路方程为 v_p-v_q-4i_n=-12。branch id=1 对应全局 k=3；
// 当前 Newton 点故意与历史完全不同，防止误读 x 而不是 context.x_prev。
TEST(InductorTransientTest, StampsBranchEquationWithNonzeroHistory) {
    MnaSystem sys(3, 2);
    Inductor inductor(2.0, 1, 2, 1);
    const std::vector<double> x{100.0, -50.0, 88.0, -99.0};
    const std::vector<double> x_prev{5.0, 2.0, 99.0, 3.0};
    const std::vector<double> x_prev_before = x_prev;
    const TransientContext context{.dt = 0.5, .x_prev = x_prev, .time = 0.5};

    inductor.stamp(sys, x, context);

    expect_system_equals(
        sys,
        {
            {0.0, 0.0, 0.0, 1.0},
            {0.0, 0.0, 0.0, -1.0},
            {0.0, 0.0, 0.0, 0.0},
            {1.0, -1.0, 0.0, -4.0},
        },
        {0.0, 0.0, 0.0, -12.0});
    EXPECT_EQ(x_prev, x_prev_before) << "stamp 不得推进或改写历史状态";
}

// 接地 oracle：p=1、q=0、L/dt=3/0.25=12ohm、i_prev=-2A，
// 所以 b(k)=-12*(-2)=+24；涉及地节点的两个耦合落点被跳过。
TEST(InductorTransientTest, GroundedNegativeTerminalSkipsOnlyNodeCouplings) {
    MnaSystem sys(2, 1);
    Inductor inductor(3.0, 1, 0, 0);
    const std::vector<double> x{100.0, 50.0};
    const std::vector<double> x_prev{8.0, -2.0};
    const TransientContext context{.dt = 0.25, .x_prev = x_prev, .time = 1.0};

    inductor.stamp(sys, x, context);

    expect_system_equals(
        sys,
        {
            {0.0, 1.0},
            {1.0, -12.0},
        },
        {0.0, 24.0});
}

// stamp 契约是 +=：同一支路模型连盖两次，五个 A 落点和历史 RHS 都恰好翻倍。
TEST(InductorTransientTest, StampAccumulatesInsteadOfOverwriting) {
    MnaSystem sys(3, 1);
    Inductor inductor(2.0, 1, 2, 0);
    const std::vector<double> x{0.0, 0.0, 0.0};
    const std::vector<double> x_prev{5.0, 2.0, 3.0};
    const TransientContext context{.dt = 0.5, .x_prev = x_prev, .time = 0.5};

    inductor.stamp(sys, x, context);
    inductor.stamp(sys, x, context);

    expect_system_equals(
        sys,
        {
            {0.0, 0.0, 2.0},
            {0.0, 0.0, -2.0},
            {2.0, -2.0, -8.0},
        },
        {0.0, 0.0, -24.0});
}

// dt 位于 L/dt 的分母；零或负步长必须响亮失败。
TEST(InductorTransientTest, RejectsNonPositiveTimeStep) {
    MnaSystem sys(2, 1);
    Inductor inductor(1.0, 1, 0, 0);
    const std::vector<double> x{0.0, 0.0};
    const std::vector<double> x_prev{0.0, 0.0};
    const TransientContext zero_dt{.dt = 0.0, .x_prev = x_prev, .time = 0.0};
    const TransientContext negative_dt{.dt = -0.1, .x_prev = x_prev, .time = 0.0};

    EXPECT_THROW(inductor.stamp(sys, x, zero_dt), std::invalid_argument);
    EXPECT_THROW(inductor.stamp(sys, x, negative_dt), std::invalid_argument);
}

// 单位串联 RL 独立 oracle：Vs=R=L=1、dt=0.1、i_0=0。
// 后向欧拉递推为 i_n=(1+10*i_prev)/11，闭式为 i_n=1-(10/11)^n。
// 解向量顺序 [v1,v2,iV,iL] 同时验证 V/L 共用支路编号池。
TEST(InductorTrajectoryTest, RlCurrentMatchesBackwardEulerClosedForm) {
    constexpr double dt = 0.1;
    MnaSystem sys(3, 2);
    VoltageSource source(1.0, 1, 0, 0);
    Resistor resistor(1.0, 1, 2);
    Inductor inductor(1.0, 2, 0, 1);
    std::vector<Device*> devices{&source, &resistor, &inductor};
    const std::vector<double> initial_x{0.0, 0.0, 0.0, 0.0};

    const std::vector<TransientPoint> trajectory =
        transient_solve(devices, sys, dt, 0.5, initial_x);

    ASSERT_EQ(trajectory.size(), 6u);
    ASSERT_EQ(trajectory[0].x, initial_x);
    EXPECT_NEAR(trajectory[1].x[3], 1.0 / 11.0, 1e-12);

    for (std::size_t n = 0; n < trajectory.size(); ++n) {
        const double expected_time = static_cast<double>(n) * dt;
        EXPECT_NEAR(trajectory[n].time, expected_time, 1e-12);
        ASSERT_EQ(trajectory[n].x.size(), 4u);

        if (n == 0) {
            continue;
        }

        const double expected_current =
            1.0 - std::pow(10.0 / 11.0, static_cast<double>(n));
        EXPECT_NEAR(trajectory[n].x[0], 1.0, 1e-12);
        EXPECT_NEAR(trajectory[n].x[1], 1.0 - expected_current, 1e-12);
        EXPECT_NEAR(trajectory[n].x[2], -expected_current, 1e-12);
        EXPECT_NEAR(trajectory[n].x[3], expected_current, 1e-12);
        EXPECT_GT(trajectory[n].x[3], trajectory[n - 1].x[3]);
        EXPECT_LE(trajectory[n].x[3], 1.0);
    }
}

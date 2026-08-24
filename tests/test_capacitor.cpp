// 电容后向欧拉 companion model 测试（AI 代写，见 docs/AI参与记录.md）
// 独立 oracle：i_n = (C/dt)(v_n-v_prev)，因此
// G_eq = C/dt，I_hist = -G_eq*v_prev；电导盖 A 四点，历史源按 p->q 盖 b 两点。
#include "devices/Capacitor.h"
#include "devices/Diode.h"
#include "devices/Resistor.h"
#include "mna/MnaSystem.h"

#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

namespace {

constexpr double kDiodeGeqAtHalfVolt = 9.708141377777516e-3;
constexpr double kDiodeIeqAtHalfVolt = -4.603095818990453e-3;

} // namespace

// 非正电容没有本任务定义下的物理意义，必须在构造期拒绝。
TEST(CapacitorConstructionTest, RejectsNonPositiveCapacitance) {
    EXPECT_THROW(Capacitor(0.0, 1, 0), std::invalid_argument);
    EXPECT_THROW(Capacitor(-2.0, 1, 0), std::invalid_argument);
}

// DC 稳态下 dv/dt=0，电容开路；用预先写脏的工作区确认 stamp 是严格 no-op，
// 而不是把调用方已经组装好的内容清掉。
TEST(CapacitorDcTest, StampIsOpenCircuitAndPreservesExistingSystem) {
    MnaSystem sys(3, 0);
    sys.add_to_A_raw(0, 0, 7.0);
    sys.add_to_A_raw(0, 1, -3.0);
    sys.add_to_A_raw(1, 0, 2.0);
    sys.add_to_A_raw(1, 1, 5.0);
    sys.add_to_b_raw(0, 11.0);
    sys.add_to_b_raw(1, -13.0);
    Capacitor c(2.0, 1, 2);

    c.stamp(sys);

    EXPECT_DOUBLE_EQ(sys.get_A()(0, 0), 7.0);
    EXPECT_DOUBLE_EQ(sys.get_A()(0, 1), -3.0);
    EXPECT_DOUBLE_EQ(sys.get_A()(1, 0), 2.0);
    EXPECT_DOUBLE_EQ(sys.get_A()(1, 1), 5.0);
    EXPECT_DOUBLE_EQ(sys.get_b()[0], 11.0);
    EXPECT_DOUBLE_EQ(sys.get_b()[1], -13.0);
}

// 手算 oracle：C=2F、dt=0.5s、v_prev=5-2=3V，
// G_eq=4S、I_hist=-12A，所以 A=[[4,-4],[-4,4]]、b=[12,-12]。
// 当前 Newton 迭代点故意取成完全不同的 {100,-50}，防止误把 x 当历史。
TEST(CapacitorTransientTest, StampsFourPointConductanceAndHistorySourceFromPreviousSolution) {
    MnaSystem sys(3, 0);
    Capacitor c(2.0, 1, 2);
    const std::vector<double> x{100.0, -50.0};
    const std::vector<double> x_prev{5.0, 2.0};
    const std::vector<double> x_prev_before = x_prev;
    const TransientContext context{.dt = 0.5, .x_prev = x_prev, .time = 0.5};

    c.stamp(sys, x, context);

    EXPECT_DOUBLE_EQ(sys.get_A()(0, 0), 4.0);
    EXPECT_DOUBLE_EQ(sys.get_A()(0, 1), -4.0);
    EXPECT_DOUBLE_EQ(sys.get_A()(1, 0), -4.0);
    EXPECT_DOUBLE_EQ(sys.get_A()(1, 1), 4.0);
    EXPECT_DOUBLE_EQ(sys.get_b()[0], 12.0);
    EXPECT_DOUBLE_EQ(sys.get_b()[1], -12.0);
    EXPECT_EQ(x_prev, x_prev_before) << "stamp 不得推进或改写历史状态";
}

// 接地 oracle：p=1、q=0、v_prev=3V，仍有 G_eq=4、I_hist=-12；
// 所有涉及地节点的落点被 MnaSystem 跳过，只留下 A(0,0)=4、b(0)=12。
TEST(CapacitorTransientTest, GroundedNegativeTerminalLeavesOneDiagonalAndOneRhsEntry) {
    MnaSystem sys(2, 0);
    Capacitor c(2.0, 1, 0);
    const std::vector<double> x{-100.0};
    const std::vector<double> x_prev{3.0};
    const TransientContext context{.dt = 0.5, .x_prev = x_prev, .time = 1.0};

    c.stamp(sys, x, context);

    EXPECT_DOUBLE_EQ(sys.get_A()(0, 0), 4.0);
    EXPECT_DOUBLE_EQ(sys.get_b()[0], 12.0);
}

// stamp 契约是 +=：同一 companion model 连盖两次，所有贡献必须恰好翻倍。
TEST(CapacitorTransientTest, StampAccumulatesInsteadOfOverwriting) {
    MnaSystem sys(3, 0);
    Capacitor c(2.0, 1, 2);
    const std::vector<double> x{0.0, 0.0};
    const std::vector<double> x_prev{5.0, 2.0};
    const TransientContext context{.dt = 0.5, .x_prev = x_prev, .time = 0.5};

    c.stamp(sys, x, context);
    c.stamp(sys, x, context);

    EXPECT_DOUBLE_EQ(sys.get_A()(0, 0), 8.0);
    EXPECT_DOUBLE_EQ(sys.get_A()(0, 1), -8.0);
    EXPECT_DOUBLE_EQ(sys.get_A()(1, 0), -8.0);
    EXPECT_DOUBLE_EQ(sys.get_A()(1, 1), 8.0);
    EXPECT_DOUBLE_EQ(sys.get_b()[0], 24.0);
    EXPECT_DOUBLE_EQ(sys.get_b()[1], -24.0);
}

// dt 位于分母；零或负步长必须响亮失败，不能产生 inf、负等效电导或静默错误。
TEST(CapacitorTransientTest, RejectsNonPositiveTimeStep) {
    MnaSystem sys(2, 0);
    Capacitor c(1.0, 1, 0);
    const std::vector<double> x{0.0};
    const std::vector<double> x_prev{0.0};
    const TransientContext zero_dt{.dt = 0.0, .x_prev = x_prev, .time = 0.0};
    const TransientContext negative_dt{.dt = -0.1, .x_prev = x_prev, .time = 0.0};

    EXPECT_THROW(c.stamp(sys, x, zero_dt), std::invalid_argument);
    EXPECT_THROW(c.stamp(sys, x, negative_dt), std::invalid_argument);
}

// Device 的 context 默认实现必须继续把线性器件转发到原 DC stamp。
TEST(TransientContextDispatchTest, LinearDeviceFallsBackToExistingStamp) {
    MnaSystem sys(3, 0);
    Resistor r(2.0, 1, 2);
    const Device* device = &r;
    const std::vector<double> x{9.0, 8.0};
    const std::vector<double> x_prev{7.0, 6.0};
    const TransientContext context{.dt = 0.25, .x_prev = x_prev, .time = 3.0};

    device->stamp(sys, x, context);

    EXPECT_DOUBLE_EQ(sys.get_A()(0, 0), 0.5);
    EXPECT_DOUBLE_EQ(sys.get_A()(0, 1), -0.5);
    EXPECT_DOUBLE_EQ(sys.get_A()(1, 0), -0.5);
    EXPECT_DOUBLE_EQ(sys.get_A()(1, 1), 0.5);
}

// 默认 context 转发还必须保住 Diode 的双参虚分派：二极管读当前 x=0.5V，
// 不能误读历史 x_prev=-0.5V。
TEST(TransientContextDispatchTest, DiodeFallsBackToCurrentNewtonIterate) {
    MnaSystem sys(2, 0);
    Diode d(1e-12, 0.025852, 1, 0);
    const Device* device = &d;
    const std::vector<double> x{0.5};
    const std::vector<double> x_prev{-0.5};
    const TransientContext context{.dt = 0.25, .x_prev = x_prev, .time = 3.0};

    device->stamp(sys, x, context);

    EXPECT_NEAR(sys.get_A()(0, 0), kDiodeGeqAtHalfVolt, 1e-12);
    EXPECT_NEAR(sys.get_b()[0], -kDiodeIeqAtHalfVolt, 1e-12);
}

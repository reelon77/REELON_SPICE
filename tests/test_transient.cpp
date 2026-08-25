// 瞬态轨迹测试（AI 代写，见 docs/AI参与记录.md）
// 数值 oracle 独立来自后向欧拉 RC 递推与连续时间解析解。
#include "solver/transient.h"

#include "devices/Capacitor.h"
#include "devices/Resistor.h"
#include "devices/VoltageSource.h"
#include "mna/MnaSystem.h"

#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>
#include <vector>

namespace {

std::vector<TransientPoint> solve_unit_rc(double t_step, double t_stop) {
    MnaSystem sys(3, 1);
    VoltageSource source(1.0, 1, 0, 0);
    Resistor resistor(1.0, 1, 2);
    Capacitor capacitor(1.0, 2, 0);
    std::vector<Device*> devices{&source, &resistor, &capacitor};
    const std::vector<double> initial_x{0.0, 0.0, 0.0};

    return transient_solve(devices, sys, t_step, t_stop, initial_x);
}

// 独立方程 A=1、b=x_prev+1。每个时间步从 x_prev 出发时：
// 第一次 Newton 求得 x_prev+1，第二次方程不变并以 diff=0 收敛。
class HistoryRecordingDevice final : public Device {
public:
    void stamp(MnaSystem&) const override {
        throw std::logic_error("transient solve must not use the DC stamp path");
    }

    void stamp(
        MnaSystem& mna,
        const std::vector<double>&,
        const TransientContext& context) const override {
        times_.push_back(context.time);
        history_values_.push_back(context.x_prev.at(0));
        mna.add_to_A(1, 1, 1.0);
        mna.add_to_b(1, context.x_prev.at(0) + 1.0);
    }

    const std::vector<double>& times() const { return times_; }
    const std::vector<double>& history_values() const { return history_values_; }

private:
    mutable std::vector<double> times_;
    mutable std::vector<double> history_values_;
};

// 第一步方程 x=x_prev，给定初值立即收敛；第二步方程 x=x_prev+1，
// max_iter=1 时必然因 diff=1 而不收敛。
class SecondStepNonconvergentDevice final : public Device {
public:
    void stamp(MnaSystem&) const override {
        throw std::logic_error("transient solve must not use the DC stamp path");
    }

    void stamp(
        MnaSystem& mna,
        const std::vector<double>&,
        const TransientContext& context) const override {
        mna.add_to_A(1, 1, 1.0);
        const double increment = context.time < 1.5 ? 0.0 : 1.0;
        mna.add_to_b(1, context.x_prev.at(0) + increment);
    }
};

} // namespace

TEST(TransientTrajectoryTest, ZeroStopReturnsOnlyInitialPoint) {
    MnaSystem sys(2, 0);
    HistoryRecordingDevice device;
    std::vector<Device*> devices{&device};
    const std::vector<double> initial_x{7.0};

    const std::vector<TransientPoint> trajectory =
        transient_solve(devices, sys, 0.1, 0.0, initial_x);

    ASSERT_EQ(trajectory.size(), 1u);
    EXPECT_DOUBLE_EQ(trajectory[0].time, 0.0);
    EXPECT_EQ(trajectory[0].x, initial_x);
    EXPECT_TRUE(device.times().empty()) << "t_stop=0 时不得偷偷执行 Newton";
}

TEST(TransientValidationTest, RejectsInvalidArguments) {
    MnaSystem sys(2, 0);
    std::vector<Device*> devices;
    const std::vector<double> initial_x{0.0};

    EXPECT_THROW(transient_solve(devices, sys, 0.0, 1.0, initial_x), std::invalid_argument);
    EXPECT_THROW(transient_solve(devices, sys, -0.1, 1.0, initial_x), std::invalid_argument);
    EXPECT_THROW(transient_solve(devices, sys, 0.1, -1.0, initial_x), std::invalid_argument);

    const std::vector<double> wrong_size;
    EXPECT_THROW(transient_solve(devices, sys, 0.1, 0.0, wrong_size), std::invalid_argument)
        << "即使不进入 Newton，也必须检查 initial_x 维度";
}

TEST(TransientValidationTest, AppliesOneEminus12RatioTolerance) {
    MnaSystem sys(2, 0);
    HistoryRecordingDevice device;
    std::vector<Device*> devices{&device};
    const std::vector<double> initial_x{0.0};

    const std::vector<TransientPoint> accepted =
        transient_solve(devices, sys, 1.0, 3.0 + 5e-13, initial_x);
    ASSERT_EQ(accepted.size(), 4u);
    EXPECT_NEAR(accepted.back().time, 3.0, 1e-15);

    EXPECT_THROW(
        transient_solve(devices, sys, 1.0, 3.0 + 2e-12, initial_x),
        std::invalid_argument);
    EXPECT_THROW(
        transient_solve(devices, sys, 0.1, 0.25, initial_x),
        std::invalid_argument);
}

TEST(TransientHistoryTest, AdvancesHistoryOnlyAfterStepConverges) {
    MnaSystem sys(2, 0);
    HistoryRecordingDevice device;
    std::vector<Device*> devices{&device};
    const std::vector<double> initial_x{0.0};

    const std::vector<TransientPoint> trajectory =
        transient_solve(devices, sys, 1.0, 2.0, initial_x);

    ASSERT_EQ(trajectory.size(), 3u);
    EXPECT_EQ(trajectory[0].x, (std::vector<double>{0.0}));
    EXPECT_EQ(trajectory[1].x, (std::vector<double>{1.0}));
    EXPECT_EQ(trajectory[2].x, (std::vector<double>{2.0}));
    EXPECT_EQ(device.times(), (std::vector<double>{1.0, 1.0, 2.0, 2.0}));
    EXPECT_EQ(device.history_values(), (std::vector<double>{0.0, 0.0, 1.0, 1.0}));
}

TEST(TransientRcTest, TrajectoryMatchesBackwardEulerClosedForm) {
    constexpr double dt = 0.1;
    const std::vector<TransientPoint> trajectory = solve_unit_rc(dt, 0.5);

    ASSERT_EQ(trajectory.size(), 6u);
    ASSERT_EQ(trajectory[0].x, (std::vector<double>{0.0, 0.0, 0.0}));
    EXPECT_NEAR(trajectory[1].x[1], 1.0 / 11.0, 1e-12);

    for (std::size_t n = 0; n < trajectory.size(); ++n) {
        const double expected_time = static_cast<double>(n) * dt;
        const double expected_vc = 1.0 - std::pow(10.0 / 11.0, static_cast<double>(n));
        EXPECT_NEAR(trajectory[n].time, expected_time, 1e-12);
        ASSERT_EQ(trajectory[n].x.size(), 3u);
        EXPECT_NEAR(trajectory[n].x[1], expected_vc, 1e-12);
        EXPECT_GE(trajectory[n].x[1], 0.0);
        EXPECT_LE(trajectory[n].x[1], 1.0);
        if (n > 0) {
            EXPECT_GT(trajectory[n].x[1], trajectory[n - 1].x[1]);
        }
    }
}

TEST(TransientRcTest, SmallerStepApproachesExactRcSolution) {
    const std::vector<TransientPoint> coarse = solve_unit_rc(0.2, 1.0);
    const std::vector<TransientPoint> fine = solve_unit_rc(0.1, 1.0);

    const double coarse_oracle = 1.0 - std::pow(1.0 / 1.2, 5.0);
    const double fine_oracle = 1.0 - std::pow(1.0 / 1.1, 10.0);
    const double exact = 1.0 - std::exp(-1.0);
    ASSERT_FALSE(coarse.empty());
    ASSERT_FALSE(fine.empty());
    EXPECT_NEAR(coarse.back().x[1], coarse_oracle, 1e-12);
    EXPECT_NEAR(fine.back().x[1], fine_oracle, 1e-12);
    EXPECT_LT(
        std::abs(fine.back().x[1] - exact),
        std::abs(coarse.back().x[1] - exact));
}

TEST(TransientFailureTest, PropagatesNewtonFailureInsteadOfReturningPartialTrajectory) {
    MnaSystem sys(2, 0);
    SecondStepNonconvergentDevice device;
    std::vector<Device*> devices{&device};
    const std::vector<double> initial_x{0.0};

    EXPECT_THROW(
        transient_solve(devices, sys, 1.0, 2.0, initial_x, 1e-9, 1),
        std::runtime_error);
}

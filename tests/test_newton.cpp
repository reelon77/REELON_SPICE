// 牛顿求解器测试(AI 代写,见 docs/AI参与记录.md)
// 数值依据:Day4 交接单实算 —— 真解 v2 = 0.574191503 V,回路电流 4.425808 mA;
// 初值全 0 时约 178 次迭代(个位数差异正常,量级不对才是 bug)。
#include "solver/newton.h"
#include "devices/Capacitor.h"
#include "devices/Resistor.h"
#include "devices/VoltageSource.h"
#include "devices/Diode.h"
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

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

// ============================================================
// 瞬态第 2A 档 · 显式初值
// 同一线性分压电路的手算真解是 {v1=10V, v2=8V, iV1=-2mA}。
// 若 Newton 确实使用调用方初值，第一轮线性求解仍得到同一向量，diff=0，恰好 1 轮收敛。
// ============================================================

TEST(NewtonInitialGuessTest, ExactSolutionAsInitialGuessConvergesInOneIteration) {
    MnaSystem sys(3, 1);
    VoltageSource v1(10.0, 1, 0, 0);
    Resistor r1(1000.0, 1, 2);
    Resistor r2(4000.0, 2, 0);
    std::vector<Device*> devices{&v1, &r1, &r2};
    const std::vector<double> initial_x{10.0, 8.0, -2e-3};

    const NewtonResult result = newton_solve(devices, sys, initial_x);

    EXPECT_EQ(result.iterations, 1) << "真解作为初值时，第一轮 diff 应为 0";
    ASSERT_EQ(result.x.size(), 3u);
    EXPECT_NEAR(result.x[0], 10.0, 1e-9);
    EXPECT_NEAR(result.x[1], 8.0, 1e-9);
    EXPECT_NEAR(result.x[2], -2e-3, 1e-9);
}

// sys.dim()=3，但初值只有 2 项；若不在入口拒绝，后续 transform_reduce 会越界读取。
TEST(NewtonInitialGuessTest, RejectsInitialGuessWithWrongDimension) {
    MnaSystem sys(3, 1);
    VoltageSource v1(10.0, 1, 0, 0);
    Resistor r1(1000.0, 1, 2);
    Resistor r2(4000.0, 2, 0);
    std::vector<Device*> devices{&v1, &r1, &r2};
    const std::vector<double> too_short{10.0, 8.0};

    EXPECT_THROW(newton_solve(devices, sys, too_short), std::invalid_argument);
}

namespace {

// 测试专用线性器件：A=1，b=x_prev+1。初值取 x_prev，故手算第 1 轮得到
// x_prev+1，第 2 轮方程不变、diff=0。单/双参路径若被误用会直接抛异常。
class RecordingTransientDevice : public Device {
public:
    void stamp(MnaSystem&) const override {
        throw std::logic_error("transient Newton must not use the DC stamp path");
    }

    void stamp(
        MnaSystem& mna,
        const std::vector<double>& x,
        const TransientContext& context) const override {
        if (calls_ == 0) {
            first_context_ = &context;
            first_history_data_ = context.x_prev.data();
            first_history_value_ = context.x_prev.at(0);
            first_iterate_ = x.at(0);
        } else {
            same_context_ = same_context_ && (&context == first_context_);
            history_unchanged_ = history_unchanged_
                && context.x_prev.data() == first_history_data_
                && context.x_prev.at(0) == first_history_value_;
        }
        ++calls_;

        mna.add_to_A(1, 1, 1.0);
        mna.add_to_b(1, context.x_prev.at(0) + 1.0);
    }

    int calls() const { return calls_; }
    double first_iterate() const { return first_iterate_; }
    bool same_context() const { return same_context_; }
    bool history_unchanged() const { return history_unchanged_; }

private:
    mutable int calls_ = 0;
    mutable double first_iterate_ = 0.0;
    mutable const TransientContext* first_context_ = nullptr;
    mutable const double* first_history_data_ = nullptr;
    mutable double first_history_value_ = 0.0;
    mutable bool same_context_ = true;
    mutable bool history_unchanged_ = true;
};

} // namespace

TEST(NewtonTransientContextTest, ReusesFrozenContextAcrossEveryIteration) {
    MnaSystem sys(2, 0);
    RecordingTransientDevice device;
    std::vector<Device*> devices{&device};
    const std::vector<double> x_prev{7.0};
    const TransientContext context{.dt = 0.1, .x_prev = x_prev, .time = 0.1};

    const NewtonResult result = newton_solve(devices, sys, x_prev, context);

    ASSERT_EQ(result.x.size(), 1u);
    EXPECT_DOUBLE_EQ(result.x[0], 8.0) << "手算方程 1*x = x_prev+1 = 8";
    EXPECT_EQ(result.iterations, 2);
    EXPECT_EQ(device.calls(), 2);
    EXPECT_DOUBLE_EQ(device.first_iterate(), 7.0) << "首轮初值必须就是 x_prev";
    EXPECT_TRUE(device.same_context()) << "所有 Newton 轮次必须收到同一个 context 对象";
    EXPECT_TRUE(device.history_unchanged()) << "Newton 内部不得推进或替换历史";
    EXPECT_EQ(x_prev, std::vector<double>{7.0});
}

// 瞬态第 2C 档：Vs=1V、R=1Ω、C=1F、dt=0.1s、vC(0)=0。
// 节点 2 手算 KCL：(v2-1)/1 + (1/0.1)(v2-0) = 0，
// 因而 11*v2=1、v2=1/11；电压源支路电流为 -(1-v2)=-10/11 A。
TEST(NewtonTransientStepTest, BackwardEulerRcFirstStepMatchesHandCalculation) {
    MnaSystem sys(3, 1);
    VoltageSource source(1.0, 1, 0, 0);
    Resistor resistor(1.0, 1, 2);
    Capacitor capacitor(1.0, 2, 0);
    std::vector<Device*> devices{&source, &resistor, &capacitor};
    const std::vector<double> x_prev{0.0, 0.0, 0.0};
    const TransientContext context{.dt = 0.1, .x_prev = x_prev, .time = 0.1};

    const NewtonResult result = newton_solve(devices, sys, x_prev, context);

    ASSERT_EQ(result.x.size(), 3u);
    EXPECT_NEAR(result.x[0], 1.0, 1e-9);
    EXPECT_NEAR(result.x[1], 1.0 / 11.0, 1e-9);
    EXPECT_NEAR(result.x[2], -10.0 / 11.0, 1e-9);
    EXPECT_EQ(x_prev, (std::vector<double>{0.0, 0.0, 0.0}))
        << "一个 Newton 求解期间不得推进上一时间步历史";
}

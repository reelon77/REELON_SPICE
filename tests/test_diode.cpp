// Diode 线性化与 stamp 落点测试(AI 代写,见 docs/AI参与记录.md)
// 期望值来源:主会话按 g_eq = (Is/Vt)·e^(v/Vt)、I_eq = I(v) − g_eq·v 实算,
// 与 Day4 交接单的对答数(v=0.5 → 9.708141378e-3 / −4.603095819e-3)一致。
#include "devices/Diode.h"
#include "mna/MnaSystem.h"
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

namespace {

constexpr double kIs = 1e-12;
constexpr double kVt = 0.025852;

// 期望值(15 位有效数字,tol 取 1e-12 相对绰绰有余)
constexpr double kGeq05 = 9.708141377777516e-3;   // v = 0.5
constexpr double kIeq05 = -4.603095818990453e-3;  // v = 0.5

} // namespace

// ============================================================
// 第 1 档:线性化单点检查(经由 1×1 stamp 读出,阴极接地)
// 电路:结点1—D—地,x = {0.5} ⇒ v = 0.5
// ============================================================

TEST(DiodeLinearizeTest, SinglePointAtHalfVolt) {
    MnaSystem sys(2, 0); // 结点 0(地)、1,无电压源 → dim = 1
    Diode d(kIs, kVt, 1, 0);
    std::vector<double> x{0.5};

    d.stamp(sys, x);

    EXPECT_NEAR(sys.get_A()(0, 0), kGeq05, 1e-12) << "g_eq 落在 (pos,pos)";
    EXPECT_NEAR(sys.get_b()[0], -kIeq05, 1e-12) << "b[pos] 应得 −I_eq";
}

// v = 0(牛顿初值全 0 的第一轮):g_eq ≈ Is/Vt 极小但非零,I_eq 恰为 0。
// 这一档守住"初值全 0 不产生 inf/nan"(除零、溢出都会在此现形)
TEST(DiodeLinearizeTest, ZeroBiasIsFiniteAndTiny) {
    MnaSystem sys(2, 0);
    Diode d(kIs, kVt, 1, 0);
    std::vector<double> x{0.0};

    d.stamp(sys, x);

    EXPECT_NEAR(sys.get_A()(0, 0), kIs / kVt, 1e-15); // = 3.868e-11
    EXPECT_NEAR(sys.get_b()[0], 0.0, 1e-15);
    EXPECT_TRUE(std::isfinite(sys.get_A()(0, 0)));
}

// 反偏 v = −0.5:g_eq ≈ 0(1.5e-19),I_eq ≈ −Is。二极管"关断"的线性化
TEST(DiodeLinearizeTest, ReverseBiasNearlyOff) {
    MnaSystem sys(2, 0);
    Diode d(kIs, kVt, 1, 0);
    std::vector<double> x{-0.5};

    d.stamp(sys, x);

    EXPECT_NEAR(sys.get_A()(0, 0), 1.541258956079159e-19, 1e-30);
    EXPECT_NEAR(sys.get_b()[0], 9.999999189525895e-13, 1e-24); // b[pos] = −I_eq,I_eq ≈ −Is 故为正
}

// ============================================================
// 第 2 档:stamp 落点(两端均非地)
// 电路:结点1—D—结点2,x = {0.7, 0.2} ⇒ v = 0.5,期望值与第 1 档同源
// ============================================================

class DiodeStampTest : public ::testing::Test {
protected:
    MnaSystem sys{3, 0}; // 结点 0、1、2 → dim = 2
    Diode d{kIs, kVt, 1, 2};
    std::vector<double> x{0.7, 0.2};
};

TEST_F(DiodeStampTest, ConductanceFourPointPattern) {
    d.stamp(sys, x);
    const Matrix& A = sys.get_A();

    EXPECT_NEAR(A(0, 0), kGeq05, 1e-12) << "(pos,pos) 自己对自己 +";
    EXPECT_NEAR(A(0, 1), -kGeq05, 1e-12) << "(pos,neg) 自己对对方 −";
    EXPECT_NEAR(A(1, 0), -kGeq05, 1e-12) << "(neg,pos) −";
    EXPECT_NEAR(A(1, 1), kGeq05, 1e-12) << "(neg,neg) +";
}

// 拉普拉斯行和性质:电导块每行之和为 0(无接地端时)
TEST_F(DiodeStampTest, ConductanceRowsSumToZero) {
    d.stamp(sys, x);
    const Matrix& A = sys.get_A();

    EXPECT_NEAR(A(0, 0) + A(0, 1), 0.0, 1e-18);
    EXPECT_NEAR(A(1, 0) + A(1, 1), 0.0, 1e-18);
}

TEST_F(DiodeStampTest, CurrentSourceTwoPointPattern) {
    d.stamp(sys, x);

    EXPECT_NEAR(sys.get_b()[0], -kIeq05, 1e-12) << "b[pos] −I_eq(抽走)";
    EXPECT_NEAR(sys.get_b()[1], kIeq05, 1e-12) << "b[neg] +I_eq(注入)";
}

// stamp 是 +=:同一器件盖两次,数值应翻倍(守住"累加而非覆盖"的语义)
TEST_F(DiodeStampTest, StampAccumulates) {
    d.stamp(sys, x);
    d.stamp(sys, x);

    EXPECT_NEAR(sys.get_A()(0, 0), 2 * kGeq05, 1e-12);
    EXPECT_NEAR(sys.get_b()[0], -2 * kIeq05, 1e-12);
}

// ============================================================
// 接口守卫:单参 stamp 必须响亮地失败,不能安静地盖错章
// ============================================================

TEST(DiodeInterfaceTest, SingleArgStampThrows) {
    MnaSystem sys(2, 0);
    Diode d(kIs, kVt, 1, 0);

    EXPECT_THROW(d.stamp(sys), std::logic_error);
}

// 经基类指针调用双参版,应落到 Diode 的实现(虚表分派正确接线)
TEST(DiodeInterfaceTest, TwoArgStampDispatchesThroughBasePointer) {
    MnaSystem sys(2, 0);
    Diode d(kIs, kVt, 1, 0);
    const Device* base = &d;
    std::vector<double> x{0.5};

    base->stamp(sys, x);

    EXPECT_NEAR(sys.get_A()(0, 0), kGeq05, 1e-12);
}

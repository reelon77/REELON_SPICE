// 解析器第 1 档:SPICE 数值解析(AI 代写,见 docs/AI参与记录.md)
// 受测契约:
//   double string2double(const std::string& str)
//   - 入参只读;输入已由切词层保证小写
//   - 裸数字/科学计数法直接收;可带一个尾部倍率后缀
//   - 后缀表: f p n u m k meg g t(注意 m=milli=1e-3,Mega 拼作 meg)
//   - 数字后既非空也非合法后缀 → std::invalid_argument,消息含原始输入
// 期望值全部由主会话手工核算(均为 10 的整数次幂缩放,无舍入陷阱)。
#include "parser/numeric.h"
#include "parser/tokenize.h"
#include <cmath>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vector>

namespace {

// 统一相对容差 1e-12:实现无论用字面量还是 pow,1 ulp 级差异都能容住
void expect_parses(const std::string& in, double expected) {
    double got = string2double(in);
    EXPECT_NEAR(got, expected, std::abs(expected) * 1e-12)
        << "输入 \"" << in << "\"";
}

} // namespace

// ---------- 裸数字:stod 原有能力不能被后缀逻辑破坏 ----------

TEST(SpiceValueTest, PlainNumbersPassThrough) {
    expect_parses("10", 10.0);
    expect_parses("4.7", 4.7);
    expect_parses("0.574191503", 0.574191503);
    expect_parses("-3", -3.0);
}

TEST(SpiceValueTest, ScientificNotationPassesThrough) {
    expect_parses("1e-12", 1e-12);   // 二极管 Is 的写法,必须活着
    expect_parses("-2.5e3", -2500.0);
}

// ---------- 单字符后缀 ----------

TEST(SpiceValueTest, KiloSuffix) {
    expect_parses("1k", 1000.0);     // 金标准网表里的 1k
    expect_parses("4k", 4000.0);     // 金标准网表里的 4k
    expect_parses("4.7k", 4700.0);   // 小数 × 倍率,字符串接零法在此现形
}

TEST(SpiceValueTest, ShrinkingSuffixes) {
    expect_parses("4.7u", 4.7e-6);
    expect_parses("100n", 1e-7);
    expect_parses("2p", 2e-12);
    expect_parses("1f", 1e-15);
}

TEST(SpiceValueTest, GrowingSuffixes) {
    expect_parses("2g", 2e9);
    expect_parses("1t", 1e12);
}

// ---------- 行业陷阱:m 是 milli,Mega 拼作 meg ----------

TEST(SpiceValueTest, MilliVersusMegaTrap) {
    expect_parses("1m", 1e-3);       // 不是 1e6!
    expect_parses("1meg", 1e6);
    expect_parses("2.2meg", 2.2e6);  // 多字符后缀整体识别
}

// ---------- 错误路径:垃圾要响亮地死 ----------

TEST(SpiceValueTest, RejectsGarbage) {
    EXPECT_THROW(string2double("abc"), std::invalid_argument);  // 无数字
    EXPECT_THROW(string2double("k1"), std::invalid_argument);   // 后缀在前
    EXPECT_THROW(string2double(""), std::invalid_argument);     // 空串
}

TEST(SpiceValueTest, RejectsBadSuffix) {
    EXPECT_THROW(string2double("1x"), std::invalid_argument);      // 表外字母
    EXPECT_THROW(string2double("1kk"), std::invalid_argument);     // 后缀重复
    EXPECT_THROW(string2double("10kohm"), std::invalid_argument);  // 垃圾尾巴
    EXPECT_THROW(string2double("1me"), std::invalid_argument);     // meg 拼一半
}

// ---------- 解析器第 2 档:单行切词 ----------
// 本组测试由 AI 代写，受测的 tokenize 实现由用户亲手编写。

TEST(TokenizeTest, EmptyAndWhitespaceOnlyLinesProduceNoTokens) {
    EXPECT_TRUE(tokenize("").empty());
    EXPECT_TRUE(tokenize("   \t\r\n").empty());
}

TEST(TokenizeTest, FullLineCommentsProduceNoTokens) {
    EXPECT_TRUE(tokenize("* comment").empty());
    EXPECT_TRUE(tokenize("   \t* indented comment").empty());
}

TEST(TokenizeTest, SplitsOnMixedWhitespaceAndTrimsEdges) {
    const std::vector<std::string> expected{"v1", "in", "0", "5"};
    EXPECT_EQ(tokenize("  \tV1  IN\t0  5\r"), expected);
}

TEST(TokenizeTest, LowercasesEveryToken) {
    const std::vector<std::string> expected{"rload", "in", "out", "1k", "1e-12"};
    EXPECT_EQ(tokenize("RLOAD IN Out 1K 1E-12"), expected);
}

TEST(TokenizeTest, PreservesDirectiveTokens) {
    EXPECT_EQ(tokenize(".OP"), std::vector<std::string>{".op"});
    EXPECT_EQ(tokenize("  .END  "), std::vector<std::string>{".end"});
}

TEST(TokenizeTest, MiddleAsteriskRemainsAnOrdinaryToken) {
    const std::vector<std::string> expected{"r1", "1", "0", "1k", "*", "text"};
    EXPECT_EQ(tokenize("R1 1 0 1K * TEXT"), expected);
}

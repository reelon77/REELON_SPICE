// 解析器第 1 档:SPICE 数值解析(AI 代写,见 docs/AI参与记录.md)
// 受测契约:
//   double string2double(const std::string& str)
//   - 入参只读;输入已由切词层保证小写
//   - 裸数字/科学计数法直接收;可带一个尾部倍率后缀
//   - 后缀表: f p n u m k meg g t(注意 m=milli=1e-3,Mega 拼作 meg)
//   - 数字后既非空也非合法后缀 → std::invalid_argument,消息含原始输入
// 期望值全部由主会话手工核算(均为 10 的整数次幂缩放,无舍入陷阱)。
#include "parser/Circuit.h"
#include "parser/numeric.h"
#include "parser/tokenize.h"
#include "devices/Capacitor.h"
#include "devices/CurrentSource.h"
#include "devices/Diode.h"
#include "devices/Inductor.h"
#include "devices/Resistor.h"
#include "devices/VoltageSource.h"
#include <cmath>
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>
#include <string>
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

// ---------- 解析器语义层第 1 档:逐行主循环 + .op/.end ----------
// 本组测试由 AI 代写，受测的 Circuit/parse_circuit 实现由用户亲手编写。

TEST(CircuitParserTest, ParsesOpWhileSkippingBlankAndCommentLines) {
    std::istringstream input(
        "\n"
        "  * comment\n"
        "\t\n"
        ".OP\n"
        ".END\n");

    Circuit circuit = parse_circuit(input);

    EXPECT_EQ(circuit.analysis_type, AnalysisType::Op);
    EXPECT_TRUE(circuit.devices.empty());
    EXPECT_EQ(circuit.nodes, 1);
    EXPECT_EQ(circuit.num_branch_unknowns, 0);
}

TEST(CircuitParserTest, EndStopsBeforeInvalidFollowingContent) {
    std::istringstream input(
        ".end\n"
        ".op unexpected\n");

    Circuit circuit = parse_circuit(input);

    EXPECT_EQ(circuit.analysis_type, AnalysisType::None);
}

TEST(CircuitParserTest, RejectsOpArgumentsAndReportsLineNumber) {
    std::istringstream input(
        "\n"
        "* comment\n"
        "\n"
        ".op unexpected\n");

    try {
        (void)parse_circuit(input);
        FAIL() << "带参数的 .op 应抛出异常";
    } catch (const std::runtime_error& e) {
        const std::string message = e.what();
        EXPECT_NE(message.find("4:"), std::string::npos);
        EXPECT_NE(message.find(".op"), std::string::npos);
    }
}

TEST(CircuitParserTest, RejectsEndArgumentsAndReportsLineNumber) {
    std::istringstream input(
        ".op\n"
        "\n"
        ".end unexpected\n");

    try {
        (void)parse_circuit(input);
        FAIL() << "带参数的 .end 应抛出异常";
    } catch (const std::runtime_error& e) {
        const std::string message = e.what();
        EXPECT_NE(message.find("3:"), std::string::npos);
        EXPECT_NE(message.find(".end"), std::string::npos);
    }
}

// ---------- 解析器语义层第 2 档:节点名称映射 ----------
// 本组测试由 AI 代写，受测的节点映射实现由用户亲手编写。

TEST(CircuitNodeMappingTest, ZeroAndGndShareTheGroundNode) {
    std::istringstream input(
        "R1 0 gnd 1k\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);

    EXPECT_EQ(circuit.nodes, 1);
}

TEST(CircuitNodeMappingTest, RepeatedNamesReuseExistingNodeNumbers) {
    std::istringstream input(
        "R1 in out 1k\n"
        "R2 0 gnd 2k\n"
        "R3 in 0 3k\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);

    EXPECT_EQ(circuit.nodes, 3);
}

TEST(CircuitNodeMappingTest, EverySupportedDevicePrefixContributesNodes) {
    std::istringstream input(
        "R1 nr 0 1k\n"
        "V1 nv 0 5\n"
        "I1 ni 0 1m\n"
        "D1 nd 0\n"
        "C1 nc 0 1u\n"
        "L1 nl 0 1m\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);

    EXPECT_EQ(circuit.nodes, 7);
}

// ---------- 解析器语义层第 3 档:器件行语义 ----------
// 本组测试由 AI 代写，受测的 R/V/I/D 解析实现由用户亲手编写。

namespace {

void expect_parse_error_contains(
    const std::string& netlist,
    const std::vector<std::string>& expected_fragments,
    const std::vector<std::string>& forbidden_fragments = {}) {
    std::istringstream input(netlist);
    try {
        (void)parse_circuit(input);
        FAIL() << "网表应被拒绝: " << netlist;
    } catch (const std::runtime_error& e) {
        const std::string message = e.what();
        for (const std::string& fragment : expected_fragments) {
            EXPECT_NE(message.find(fragment), std::string::npos)
                << "错误消息缺少片段 \"" << fragment << "\": " << message;
        }
        for (const std::string& fragment : forbidden_fragments) {
            EXPECT_EQ(message.find(fragment), std::string::npos)
                << "错误路径仍落入未知 token 兜底: " << message;
        }
    }
}

} // namespace

TEST(CircuitDeviceParsingTest, CreatesEverySupportedDeviceInNetlistOrder) {
    std::istringstream input(
        "R1 nr 0 1k\n"
        "V1 nv 0 5\n"
        "I1 ni 0 1m\n"
        "D1 nd 0\n"
        "C1 nc 0 1u\n"
        "L1 nl 0 1m\n"
        ".op\n"
        ".end\n");

    Circuit circuit = parse_circuit(input);

    ASSERT_EQ(circuit.devices.size(), 6u);
    EXPECT_NE(dynamic_cast<Resistor*>(circuit.devices[0].get()), nullptr);
    EXPECT_NE(dynamic_cast<VoltageSource*>(circuit.devices[1].get()), nullptr);
    EXPECT_NE(dynamic_cast<CurrentSource*>(circuit.devices[2].get()), nullptr);
    EXPECT_NE(dynamic_cast<Diode*>(circuit.devices[3].get()), nullptr);
    EXPECT_NE(dynamic_cast<Capacitor*>(circuit.devices[4].get()), nullptr);
    EXPECT_NE(dynamic_cast<Inductor*>(circuit.devices[5].get()), nullptr);
    EXPECT_EQ(circuit.nodes, 7);
    EXPECT_EQ(circuit.num_branch_unknowns, 2);
    EXPECT_EQ(circuit.analysis_type, AnalysisType::Op);
}

TEST(CircuitDeviceParsingTest, RejectsUnknownDeviceAndReportsLineAndToken) {
    expect_parse_error_contains(
        "\nX1 1 0 7\n",
        {"2:", "x1"});
}

TEST(CircuitDeviceParsingTest, RejectsUnsupportedDirectiveAndReportsLineAndToken) {
    expect_parse_error_contains(
        ".dc V1 0 5 1\n",
        {"1:", ".dc"});
}

TEST(CircuitDeviceParsingTest, RejectsWrongTokenCountForEveryDeviceKind) {
    const std::vector<std::string> malformed_lines{
        "R1 1 0\n",
        "V1 1 0 5 extra\n",
        "I1 1 0\n",
        "D1 1 0 model\n",
        "C1 1 0\n",
        "L1 1 0 1m extra\n",
    };

    for (const std::string& line : malformed_lines) {
        SCOPED_TRACE(line);
        expect_parse_error_contains(line, {"1:"}, {"undefined token"});
    }
}

TEST(CircuitDeviceParsingTest, RejectsInvalidNumericValueWithLineAndToken) {
    const std::vector<std::string> invalid_numeric_lines{
        "R1 1 0 invalid\n",
        "V1 1 0 invalid\n",
        "I1 1 0 invalid\n",
        "C1 1 0 invalid\n",
        "L1 1 0 invalid\n",
    };

    for (const std::string& line : invalid_numeric_lines) {
        SCOPED_TRACE(line);
        expect_parse_error_contains(
            line,
            {"1:", "invalid"},
            {"undefined token"});
    }
}

// ---------- 瞬态第 5 档:C/L/.tran 语义 ----------
// 本组测试由 AI 代写；Circuit/parser 实现由用户亲手编写。

TEST(CircuitTransientParsingTest, ParsesCapacitorInductorAndTranCaseInsensitively) {
    std::istringstream input(
        "cStore cnode 0 500M\n"
        "LStore lnode 0 500M\n"
        "vDrive vnode 0 2\n"
        ".TrAn 250M 500M\n"
        ".EnD\n");

    Circuit circuit = parse_circuit(input);

    ASSERT_EQ(circuit.devices.size(), 3u);
    EXPECT_NE(dynamic_cast<Capacitor*>(circuit.devices[0].get()), nullptr);
    EXPECT_NE(dynamic_cast<Inductor*>(circuit.devices[1].get()), nullptr);
    EXPECT_NE(dynamic_cast<VoltageSource*>(circuit.devices[2].get()), nullptr);
    EXPECT_EQ(circuit.nodes, 4);
    EXPECT_EQ(circuit.num_branch_unknowns, 2)
        << "C 不扩维，L/V 按出现顺序共享同一个支路编号池";
    EXPECT_EQ(circuit.analysis_type, AnalysisType::Tran);
    EXPECT_DOUBLE_EQ(circuit.t_step, 0.25);
    EXPECT_DOUBLE_EQ(circuit.t_stop, 0.5);
}

TEST(CircuitTransientParsingTest, RejectsWrongTranTokenCountWithLineNumber) {
    const std::vector<std::string> malformed_lines{
        ".tran 0.1\n",
        ".tran 0.1 1.0 extra\n",
    };

    for (const std::string& line : malformed_lines) {
        SCOPED_TRACE(line);
        expect_parse_error_contains(
            line,
            {"1:", ".tran"},
            {"undefined token"});
    }
}

TEST(CircuitTransientParsingTest, RejectsInvalidTranNumericValuesWithLineAndToken) {
    const std::vector<std::string> malformed_lines{
        ".tran bad 1.0\n",
        ".tran 0.1 bad\n",
    };

    for (const std::string& line : malformed_lines) {
        SCOPED_TRACE(line);
        expect_parse_error_contains(
            line,
            {"1:", "bad"},
            {"undefined token"});
    }
}

TEST(CircuitTransientParsingTest, RejectsNonPositiveCapacitanceAndInductanceWithLineNumber) {
    const std::vector<std::string> malformed_lines{
        "C1 1 0 0\n",
        "C1 1 0 -1\n",
        "L1 1 0 0\n",
        "L1 1 0 -1\n",
    };

    for (const std::string& line : malformed_lines) {
        SCOPED_TRACE(line);
        expect_parse_error_contains(line, {"1:"}, {"undefined token"});
    }
}

// parser 的网表语义比底层 transient_solve 更严格：一条 .tran 指令必须真正向前推进时间，
// 因而 step/stop 都要求正值；底层 API 的 t_stop=0 单点轨迹能力继续由原测试保留。
TEST(CircuitTransientParsingTest, RejectsNonPositiveTranParametersWithLineNumber) {
    const std::vector<std::string> malformed_lines{
        ".tran 0 1\n",
        ".tran -0.1 1\n",
        ".tran 0.1 0\n",
        ".tran 0.1 -1\n",
    };

    for (const std::string& line : malformed_lines) {
        SCOPED_TRACE(line);
        expect_parse_error_contains(line, {"1:"}, {"undefined token"});
    }
}

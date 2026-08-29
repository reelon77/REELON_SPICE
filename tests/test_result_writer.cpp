// M08 result writer tests (AI-authored under the user's 2026-08-30 special authorization).
#include "output/result_writer.h"
#include "parser/Circuit.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <locale>
#include <sstream>
#include <streambuf>
#include <string>
#include <vector>

namespace {

Circuit divider_circuit() {
    Circuit circuit;
    circuit.nodes = 3;
    circuit.num_branch_unknowns = 1;
    circuit.node_names = {"0", "1", "2"};
    circuit.branch_names = {"v1"};
    return circuit;
}

Circuit rc_circuit() {
    Circuit circuit;
    circuit.nodes = 3;
    circuit.num_branch_unknowns = 1;
    circuit.node_names = {"0", "in", "out"};
    circuit.branch_names = {"v1"};
    return circuit;
}

std::vector<std::string> split_lines(const std::string& text) {
    std::istringstream input(text);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        lines.push_back(line);
    }
    return lines;
}

double parse_assignment_value(
    const std::string& line,
    const std::string& expected_label) {
    const std::string prefix = expected_label + " = ";
    EXPECT_EQ(line.substr(0, prefix.size()), prefix);
    return std::stod(line.substr(prefix.size()));
}

std::vector<double> parse_csv_row(const std::string& line) {
    std::istringstream input(line);
    std::vector<double> values;
    std::string field;
    while (std::getline(input, field, ',')) {
        values.push_back(std::stod(field));
    }
    return values;
}

class CommaDecimalPoint : public std::numpunct<char> {
protected:
    char do_decimal_point() const override {
        return ',';
    }
};

class FailingStreambuf : public std::streambuf {
protected:
    int_type overflow(int_type) override {
        return traits_type::eof();
    }

    std::streamsize xsputn(const char*, std::streamsize) override {
        return 0;
    }
};

} // namespace

TEST(ResultWriterOperatingPointTest, WritesLabelsHandOracleAndIterations) {
    Circuit circuit = divider_circuit();
    const OperatingPointResult result{{10.0, 8.0, -2e-3}, 2};
    std::ostringstream output;

    write_operating_point(output, circuit, result);

    const std::vector<std::string> lines = split_lines(output.str());
    ASSERT_EQ(lines.size(), 5u);
    EXPECT_EQ(lines[0], "analysis: .op");
    EXPECT_DOUBLE_EQ(parse_assignment_value(lines[1], "V(1)"), 10.0);
    EXPECT_DOUBLE_EQ(parse_assignment_value(lines[2], "V(2)"), 8.0);
    EXPECT_NEAR(parse_assignment_value(lines[3], "I(v1)"), -2e-3, 1e-15);
    EXPECT_EQ(lines[4], "newton_iterations = 2");
    EXPECT_EQ(output.str().back(), '\n');
}

TEST(ResultWriterTransientTest, WritesStableHeaderAndHandCalculatedRcRows) {
    Circuit circuit = rc_circuit();
    const TransientAnalysisResult result{{
        {0.0, {0.0, 0.0, 0.0}},
        {0.25, {2.0, 0.4, -0.8}},
        {0.5, {2.0, 0.72, -0.64}},
    }};
    std::ostringstream output;

    write_transient_csv(output, circuit, result);

    const std::vector<std::string> lines = split_lines(output.str());
    ASSERT_EQ(lines.size(), 4u);
    EXPECT_EQ(lines[0], "time,V(in),V(out),I(v1)");

    const std::vector<std::vector<double>> expected{
        {0.0, 0.0, 0.0, 0.0},
        {0.25, 2.0, 0.4, -0.8},
        {0.5, 2.0, 0.72, -0.64},
    };
    for (std::size_t row = 0; row < expected.size(); ++row) {
        const std::vector<double> actual = parse_csv_row(lines[row + 1]);
        ASSERT_EQ(actual.size(), expected[row].size());
        for (std::size_t column = 0; column < expected[row].size(); ++column) {
            EXPECT_NEAR(actual[column], expected[row][column], 1e-12);
        }
    }
}

TEST(ResultWriterValidationTest, RejectsMetadataMismatchBeforeWriting) {
    Circuit circuit = divider_circuit();
    circuit.node_names.pop_back();
    std::ostringstream output;

    EXPECT_THROW(
        write_operating_point(output, circuit, {{10.0, 8.0, -2e-3}, 1}),
        std::invalid_argument);
    EXPECT_TRUE(output.str().empty());
}

TEST(ResultWriterValidationTest, RejectsEveryResultDimensionMismatchBeforeWriting) {
    Circuit circuit = rc_circuit();
    std::ostringstream output;

    EXPECT_THROW(
        write_operating_point(output, circuit, {{2.0, 0.4}, 1}),
        std::invalid_argument);
    EXPECT_TRUE(output.str().empty());

    const TransientAnalysisResult transient{{
        {0.0, {0.0, 0.0, 0.0}},
        {0.25, {2.0, 0.4}},
    }};
    EXPECT_THROW(
        write_transient_csv(output, circuit, transient),
        std::invalid_argument);
    EXPECT_TRUE(output.str().empty());
}

TEST(ResultWriterValidationTest, RejectsEmptyTrajectoryAndUnsupportedCsvName) {
    Circuit circuit = rc_circuit();
    std::ostringstream output;

    EXPECT_THROW(
        write_transient_csv(output, circuit, TransientAnalysisResult{}),
        std::invalid_argument);
    EXPECT_TRUE(output.str().empty());

    circuit.node_names[1] = "in,bad";
    const TransientAnalysisResult result{{{0.0, {0.0, 0.0, 0.0}}}};
    EXPECT_THROW(
        write_transient_csv(output, circuit, result),
        std::invalid_argument);
    EXPECT_TRUE(output.str().empty());
}

TEST(ResultWriterFormatTest, UsesClassicLocaleWithoutChangingCallerState) {
    Circuit circuit = rc_circuit();
    const TransientAnalysisResult result{{{0.25, {2.0, 0.4, -0.8}}}};
    std::ostringstream output;
    const std::locale custom_locale(
        std::locale::classic(),
        new CommaDecimalPoint);
    output.imbue(custom_locale);
    output.setf(std::ios::scientific);
    output.setf(std::ios::showpos);
    output.precision(2);
    const std::ios::fmtflags original_flags = output.flags();
    const std::streamsize original_precision = output.precision();
    const std::locale original_locale = output.getloc();

    write_transient_csv(output, circuit, result);

    const std::vector<std::string> lines = split_lines(output.str());
    ASSERT_EQ(lines.size(), 2u);
    EXPECT_EQ(lines[0], "time,V(in),V(out),I(v1)");
    const std::vector<double> row = parse_csv_row(lines[1]);
    ASSERT_EQ(row.size(), 4u);
    EXPECT_DOUBLE_EQ(row[0], 0.25);
    EXPECT_DOUBLE_EQ(row[2], 0.4);
    EXPECT_EQ(output.flags(), original_flags);
    EXPECT_EQ(output.precision(), original_precision);
    EXPECT_EQ(output.getloc(), original_locale);
}

TEST(ResultWriterStreamTest, ThrowsWhenDestinationCannotAcceptWrites) {
    Circuit circuit = divider_circuit();
    const OperatingPointResult result{{10.0, 8.0, -2e-3}, 2};
    FailingStreambuf buffer;
    std::ostream output(&buffer);

    EXPECT_THROW(
        write_operating_point(output, circuit, result),
        std::runtime_error);
}

TEST(ResultWriterDispatchTest, RoutesBothVariantAlternatives) {
    Circuit circuit = rc_circuit();
    std::ostringstream op_output;
    std::ostringstream transient_output;

    write_simulation_result(
        op_output,
        circuit,
        OperatingPointResult{{2.0, 0.4, -0.8}, 1});
    write_simulation_result(
        transient_output,
        circuit,
        TransientAnalysisResult{{{0.0, {0.0, 0.0, 0.0}}}});

    EXPECT_NE(op_output.str().find("analysis: .op\n"), std::string::npos);
    EXPECT_NE(
        transient_output.str().find("time,V(in),V(out),I(v1)\n"),
        std::string::npos);
}

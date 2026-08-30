// M08 CLI tests (AI-authored under the user's 2026-08-30 special authorization).
#include "run_cli.h"

#include <gtest/gtest.h>

#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>
#include <system_error>
#include <vector>

namespace {

class TempWorkspace {
public:
    TempWorkspace() {
        const testing::TestInfo* info =
            testing::UnitTest::GetInstance()->current_test_info();
        std::string id = std::string(info->test_suite_name()) + "_" + info->name();
        for (char& character : id) {
            if (!std::isalnum(static_cast<unsigned char>(character))) {
                character = '_';
            }
        }
        root_ = std::filesystem::temp_directory_path() / ("tinyspice_" + id);
        std::error_code ignored;
        std::filesystem::remove_all(root_, ignored);
        std::filesystem::create_directories(root_);
    }

    ~TempWorkspace() {
        std::error_code ignored;
        std::filesystem::remove_all(root_, ignored);
    }

    std::filesystem::path write(const std::string& name, const std::string& text) {
        const std::filesystem::path path = root_ / name;
        std::ofstream output(path);
        output << text;
        output.close();
        if (!output) {
            throw std::runtime_error("failed to create CLI test input");
        }
        return path;
    }

    std::filesystem::path path(const std::string& name) const {
        return root_ / name;
    }

private:
    std::filesystem::path root_;
};

std::string read_text(const std::filesystem::path& path) {
    std::ifstream input(path);
    std::ostringstream text;
    text << input.rdbuf();
    return text.str();
}

const std::string kDividerNetlist =
    "V1 1 0 10\n"
    "R1 1 2 1k\n"
    "R2 2 0 4k\n"
    ".op\n"
    ".end\n";

const std::string kRcNetlist =
    "V1 in 0 2\n"
    "R1 in out 2\n"
    "C1 out 0 500m\n"
    ".tran 250m 500m\n"
    ".end\n";

const std::string kDcNetlist =
    "V1 in 0 0\n"
    "R1 in out 1k\n"
    "R2 out 0 1k\n"
    ".dc v1 0 2 1\n"
    ".end\n";

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

TEST(CliRunnerSuccessTest, WritesOperatingPointToStdout) {
    TempWorkspace files;
    const auto input = files.write("divider.cir", kDividerNetlist);
    std::ostringstream output;
    std::ostringstream error;

    const int code = run_cli({input.string()}, output, error);

    EXPECT_EQ(code, 0);
    EXPECT_TRUE(error.str().empty());
    EXPECT_NE(output.str().find("analysis: .op\n"), std::string::npos);
    EXPECT_NE(output.str().find("V(1) = 10\n"), std::string::npos);
    EXPECT_NE(output.str().find("V(2) = 8\n"), std::string::npos);
    const std::string current_prefix = "I(v1) = ";
    const std::size_t current_position = output.str().find(current_prefix);
    ASSERT_NE(current_position, std::string::npos);
    EXPECT_NEAR(
        std::stod(output.str().substr(current_position + current_prefix.size())),
        -2e-3,
        1e-15);
}

TEST(CliRunnerSuccessTest, WritesTransientCsvToStdout) {
    TempWorkspace files;
    const auto input = files.write("rc.cir", kRcNetlist);
    std::ostringstream output;
    std::ostringstream error;

    const int code = run_cli({input.string()}, output, error);

    EXPECT_EQ(code, 0);
    EXPECT_TRUE(error.str().empty());
    EXPECT_EQ(
        output.str().substr(0, output.str().find('\n')),
        "time,V(in),V(out),I(v1)");
    EXPECT_NE(output.str().find("0.25,2,0.40000000000000002"), std::string::npos);
    EXPECT_NE(output.str().find("0.5,2,0.71999999999999997"), std::string::npos);
}

TEST(CliRunnerSuccessTest, WritesDcSweepCsvToStdout) {
    TempWorkspace files;
    const auto input = files.write("divider-dc.cir", kDcNetlist);
    std::ostringstream output;
    std::ostringstream error;

    const int code = run_cli({input.string()}, output, error);

    EXPECT_EQ(code, 0);
    EXPECT_TRUE(error.str().empty());
    EXPECT_EQ(
        output.str().substr(0, output.str().find('\n')),
        "sweep(v1),V(in),V(out),I(v1)");
    EXPECT_NE(
        output.str().find("1,1,0.5,-0.00050000000000000001\n"),
        std::string::npos);
    EXPECT_NE(output.str().find("2,2,1,-0.001\n"), std::string::npos);
}

TEST(CliRunnerSuccessTest, WritesOutputFileWithoutDuplicatingStdout) {
    TempWorkspace files;
    const auto input = files.write("divider.cir", kDividerNetlist);
    const auto output_path = files.path("result.txt");
    std::ostringstream output;
    std::ostringstream error;

    const int code = run_cli(
        {input.string(), "-o", output_path.string()},
        output,
        error);

    EXPECT_EQ(code, 0);
    EXPECT_TRUE(output.str().empty());
    EXPECT_TRUE(error.str().empty());
    const std::string written = read_text(output_path);
    EXPECT_NE(written.find("analysis: .op\n"), std::string::npos);
    EXPECT_NE(written.find("V(2) = 8\n"), std::string::npos);
}

TEST(CliRunnerMatlabTest, WritesTransientCsvBeforeInvokingPlotRunner) {
    TempWorkspace files;
    const auto input = files.write("rc.cir", kRcNetlist);
    const auto csv_path = files.path("result.csv");
    const auto image_path = files.path("result.png");
    std::ostringstream output;
    std::ostringstream error;
    bool invoked = false;

    const int code = run_cli(
        {
            input.string(),
            "-o",
            csv_path.string(),
            "--matlab-plot",
            image_path.string(),
        },
        output,
        error,
        [&](const std::filesystem::path& actual_csv,
            const std::filesystem::path& actual_image) {
            invoked = true;
            EXPECT_EQ(actual_csv, csv_path);
            EXPECT_EQ(actual_image, image_path);
            EXPECT_EQ(
                read_text(actual_csv).substr(
                    0,
                    read_text(actual_csv).find('\n')),
                "time,V(in),V(out),I(v1)");
            std::ofstream image(actual_image);
            image << "fake MATLAB image";
        });

    EXPECT_EQ(code, 0);
    EXPECT_TRUE(invoked);
    EXPECT_TRUE(output.str().empty());
    EXPECT_TRUE(error.str().empty());
    EXPECT_EQ(read_text(image_path), "fake MATLAB image");
}

TEST(CliRunnerMatlabTest, AcceptsPlotAndOutputOptionsInEitherOrder) {
    TempWorkspace files;
    const auto input = files.write("rc.cir", kRcNetlist);
    const auto csv_path = files.path("result.csv");
    const auto image_path = files.path("result.png");
    std::ostringstream output;
    std::ostringstream error;
    bool invoked = false;

    const int code = run_cli(
        {
            input.string(),
            "--matlab-plot",
            image_path.string(),
            "-o",
            csv_path.string(),
        },
        output,
        error,
        [&](const auto&, const auto&) { invoked = true; });

    EXPECT_EQ(code, 0);
    EXPECT_TRUE(invoked);
    EXPECT_TRUE(error.str().empty());
}

TEST(CliRunnerMatlabTest, ReportsPlotFailureAndPreservesCsv) {
    TempWorkspace files;
    const auto input = files.write("rc.cir", kRcNetlist);
    const auto csv_path = files.path("result.csv");
    const auto image_path = files.path("result.png");
    std::ostringstream output;
    std::ostringstream error;

    const int code = run_cli(
        {
            input.string(),
            "-o",
            csv_path.string(),
            "--matlab-plot",
            image_path.string(),
        },
        output,
        error,
        [](const auto&, const auto&) {
            throw std::runtime_error(
                "MATLAB plotting failed with exit code 7");
        });

    EXPECT_EQ(code, 1);
    EXPECT_TRUE(output.str().empty());
    EXPECT_NE(
        error.str().find("MATLAB plotting failed with exit code 7"),
        std::string::npos);
    EXPECT_TRUE(std::filesystem::is_regular_file(csv_path));
    EXPECT_FALSE(std::filesystem::exists(image_path));
}

TEST(CliRunnerMatlabTest, RejectsNonTransientAnalysisBeforeWriting) {
    TempWorkspace files;
    const auto input = files.write("divider.cir", kDividerNetlist);
    const auto output_path = files.path("result.txt");
    const auto image_path = files.path("result.png");
    std::ostringstream output;
    std::ostringstream error;
    bool invoked = false;

    const int code = run_cli(
        {
            input.string(),
            "-o",
            output_path.string(),
            "--matlab-plot",
            image_path.string(),
        },
        output,
        error,
        [&](const auto&, const auto&) { invoked = true; });

    EXPECT_EQ(code, 1);
    EXPECT_FALSE(invoked);
    EXPECT_TRUE(output.str().empty());
    EXPECT_NE(
        error.str().find("--matlab-plot requires a .tran analysis"),
        std::string::npos);
    EXPECT_FALSE(std::filesystem::exists(output_path));
    EXPECT_FALSE(std::filesystem::exists(image_path));
}

TEST(CliRunnerUsageTest, RejectsInvalidArgumentShapesWithExitTwo) {
    const std::vector<std::vector<std::string>> cases{
        {},
        {"input.cir", "-o"},
        {"input.cir", "--matlab-plot", "plot.png"},
        {"input.cir", "-o", "out.csv", "--matlab-plot"},
        {"input.cir", "-o", "out.csv", "--matlab-plot", ""},
        {"input.cir", "-o", "one.csv", "-o", "two.csv"},
        {"input.cir", "-o", "out.csv", "--matlab-plot", "one.png",
            "--matlab-plot", "two.png"},
        {"input.cir", "--output", "out.txt"},
        {"input.cir", "-o", "out.txt", "extra"},
        {""},
        {"input.cir", "-o", ""},
    };

    for (const auto& args : cases) {
        SCOPED_TRACE(testing::PrintToString(args));
        std::ostringstream output;
        std::ostringstream error;

        EXPECT_EQ(run_cli(args, output, error), 2);
        EXPECT_TRUE(output.str().empty());
        EXPECT_NE(error.str().find("error: invalid arguments\n"), std::string::npos);
        EXPECT_NE(
            error.str().find(
                "usage: TinySpice <netlist-file> [-o <output-file> "
                "--matlab-plot <image-file>]\n"),
            std::string::npos);
    }
}

TEST(CliRunnerMatlabTest, RejectsPlotPathAliasesWithoutTouchingInputOrCsv) {
    TempWorkspace files;
    const auto input = files.write("rc.cir", kRcNetlist);
    const auto csv_path = files.path("result.csv");
    std::ostringstream output;
    std::ostringstream error;

    EXPECT_EQ(
        run_cli(
            {
                input.string(),
                "-o",
                csv_path.string(),
                "--matlab-plot",
                csv_path.string(),
            },
            output,
            error,
            [](const auto&, const auto&) {}),
        1);
    EXPECT_NE(
        error.str().find(
            "MATLAB plot file must differ from simulation output file"),
        std::string::npos);
    EXPECT_EQ(read_text(input), kRcNetlist);
    EXPECT_FALSE(std::filesystem::exists(csv_path));

    output.str("");
    output.clear();
    error.str("");
    error.clear();
    EXPECT_EQ(
        run_cli(
            {
                input.string(),
                "-o",
                csv_path.string(),
                "--matlab-plot",
                input.string(),
            },
            output,
            error,
            [](const auto&, const auto&) {}),
        1);
    EXPECT_NE(
        error.str().find("MATLAB plot file must differ from input file"),
        std::string::npos);
    EXPECT_EQ(read_text(input), kRcNetlist);
    EXPECT_FALSE(std::filesystem::exists(csv_path));
}

TEST(CliRunnerFailureTest, ReportsMissingInputPathWithExitOne) {
    TempWorkspace files;
    const auto missing = files.path("missing.cir");
    std::ostringstream output;
    std::ostringstream error;

    EXPECT_EQ(run_cli({missing.string()}, output, error), 1);
    EXPECT_TRUE(output.str().empty());
    EXPECT_NE(error.str().find("cannot open input file"), std::string::npos);
    EXPECT_NE(error.str().find(missing.string()), std::string::npos);
}

TEST(CliRunnerFailureTest, ReportsParserAndSolverFailuresWithExitOne) {
    TempWorkspace files;
    const auto invalid = files.write("invalid.cir", "X1 1 0 7\n");
    const auto singular = files.write(
        "singular.cir",
        "R1 1 2 1k\n.op\n.end\n");

    for (const auto& [path, fragment] :
         std::vector<std::pair<std::filesystem::path, std::string>>{
             {invalid, "undefined token"},
             {singular, "error:"},
         }) {
        SCOPED_TRACE(path.string());
        std::ostringstream output;
        std::ostringstream error;
        EXPECT_EQ(run_cli({path.string()}, output, error), 1);
        EXPECT_TRUE(output.str().empty());
        EXPECT_NE(error.str().find(fragment), std::string::npos);
    }
}

TEST(CliRunnerFailureTest, ReportsDcPointContextWithExitOne) {
    TempWorkspace files;
    const auto input = files.write(
        "singular-dc.cir",
        "V1 driven 0 0\n"
        "RFloat a b 1k\n"
        ".dc v1 0 1 1\n"
        ".end\n");
    std::ostringstream output;
    std::ostringstream error;

    EXPECT_EQ(run_cli({input.string()}, output, error), 1);
    EXPECT_TRUE(output.str().empty());
    EXPECT_NE(error.str().find("DC sweep point 1"), std::string::npos);
    EXPECT_NE(error.str().find("v1=0"), std::string::npos);
}

TEST(CliRunnerFailureTest, RejectsSameInputAndOutputWithoutTruncatingInput) {
    TempWorkspace files;
    const auto input = files.write("divider.cir", kDividerNetlist);
    const std::vector<std::filesystem::path> aliases{
        input,
        input.parent_path() / "." / input.filename(),
    };

    for (const std::filesystem::path& output_path : aliases) {
        SCOPED_TRACE(output_path.string());
        std::ostringstream output;
        std::ostringstream error;

        EXPECT_EQ(
            run_cli({input.string(), "-o", output_path.string()}, output, error),
            1);
        EXPECT_TRUE(output.str().empty());
        EXPECT_NE(error.str().find("output file must differ"), std::string::npos);
        EXPECT_EQ(read_text(input), kDividerNetlist);
    }
}

TEST(CliRunnerFailureTest, ReportsOutputOpenFailureWithExitOne) {
    TempWorkspace files;
    const auto input = files.write("divider.cir", kDividerNetlist);
    const auto output_path = files.path("missing-directory/result.txt");
    std::ostringstream output;
    std::ostringstream error;

    EXPECT_EQ(
        run_cli({input.string(), "-o", output_path.string()}, output, error),
        1);
    EXPECT_TRUE(output.str().empty());
    EXPECT_NE(error.str().find("cannot open output file"), std::string::npos);
    EXPECT_NE(error.str().find(output_path.string()), std::string::npos);
}

TEST(CliRunnerFailureTest, ReportsBrokenStdoutWithExitOne) {
    TempWorkspace files;
    const auto input = files.write("divider.cir", kDividerNetlist);
    FailingStreambuf buffer;
    std::ostream output(&buffer);
    std::ostringstream error;

    EXPECT_EQ(run_cli({input.string()}, output, error), 1);
    EXPECT_NE(error.str().find("Failed to write simulation result"), std::string::npos);
}

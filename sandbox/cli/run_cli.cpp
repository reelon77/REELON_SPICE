#include "run_cli.h"

#include "matlab_plot.h"

#include "output/result_writer.h"
#include "parser/Circuit.h"
#include "sim/simulate.h"

#include <exception>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {
constexpr int kSuccess = 0;
constexpr int kRuntimeFailure = 1;
constexpr int kUsageFailure = 2;
constexpr const char* kUsage =
    "usage: TinySpice <netlist-file> [-o <output-file> "
    "--matlab-plot <image-file>]\n";

struct CliOptions {
    std::string input_path;
    std::optional<std::string> output_path;
    std::optional<std::string> matlab_plot_path;
};

std::optional<CliOptions> parse_options(
    const std::vector<std::string>& args) {
    if (args.empty() || args[0].empty()) {
        return std::nullopt;
    }

    CliOptions options{args[0], std::nullopt, std::nullopt};
    for (std::size_t index = 1; index < args.size(); index += 2) {
        if (index + 1 >= args.size() || args[index + 1].empty()) {
            return std::nullopt;
        }
        if (args[index] == "-o" && !options.output_path) {
            options.output_path = args[index + 1];
        } else if (args[index] == "--matlab-plot"
                   && !options.matlab_plot_path) {
            options.matlab_plot_path = args[index + 1];
        } else {
            return std::nullopt;
        }
    }
    if (options.matlab_plot_path && !options.output_path) {
        return std::nullopt;
    }
    return options;
}

int report_usage_error(std::ostream& error) {
    error << "error: invalid arguments\n" << kUsage;
    return kUsageFailure;
}

int report_runtime_error(std::ostream& error, const std::string& message) {
    error << "error: " << message << '\n';
    return kRuntimeFailure;
}

bool paths_refer_to_same_file(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path) {
    if (input_path == output_path) {
        return true;
    }
    std::error_code error;
    const bool equivalent =
        std::filesystem::equivalent(input_path, output_path, error);
    return !error && equivalent;
}

void write_output_file(
    const std::string& path,
    const Circuit& circuit,
    const SimulationResult& result) {
    std::ofstream output(path, std::ios::out | std::ios::trunc);
    if (!output) {
        throw std::runtime_error("cannot open output file '" + path + "'");
    }

    write_simulation_result(output, circuit, result);
    output.flush();
    if (!output) {
        throw std::runtime_error("failed to flush output file '" + path + "'");
    }
    output.close();
    if (!output) {
        throw std::runtime_error("failed to close output file '" + path + "'");
    }
}
} // namespace

int run_cli(
    const std::vector<std::string>& args,
    std::ostream& standard_out,
    std::ostream& standard_err) {
    return run_cli(
        args,
        standard_out,
        standard_err,
        run_matlab_plot);
}

int run_cli(
    const std::vector<std::string>& args,
    std::ostream& standard_out,
    std::ostream& standard_err,
    const MatlabPlotRunner& matlab_plot_runner) {
    const std::optional<CliOptions> options = parse_options(args);
    if (!options) {
        return report_usage_error(standard_err);
    }

    const std::string& input_path = options->input_path;
    if (options->output_path
        && paths_refer_to_same_file(input_path, *options->output_path)) {
        return report_runtime_error(
            standard_err,
            "output file must differ from input file '" + input_path + "'");
    }
    if (options->matlab_plot_path
        && paths_refer_to_same_file(
            input_path,
            *options->matlab_plot_path)) {
        return report_runtime_error(
            standard_err,
            "MATLAB plot file must differ from input file '"
            + input_path + "'");
    }
    if (options->matlab_plot_path
        && paths_refer_to_same_file(
            *options->output_path,
            *options->matlab_plot_path)) {
        return report_runtime_error(
            standard_err,
            "MATLAB plot file must differ from simulation output file '"
            + *options->output_path + "'");
    }

    try {
        std::ifstream input(input_path);
        if (!input) {
            return report_runtime_error(
                standard_err,
                "cannot open input file '" + input_path + "'");
        }

        Circuit circuit = parse_circuit(input);
        SimulationResult result = simulate(circuit);

        if (options->matlab_plot_path
            && !std::holds_alternative<TransientAnalysisResult>(result)) {
            throw std::invalid_argument(
                "--matlab-plot requires a .tran analysis");
        }

        if (options->output_path) {
            write_output_file(*options->output_path, circuit, result);
        } else {
            write_simulation_result(standard_out, circuit, result);
        }
        if (options->matlab_plot_path) {
            matlab_plot_runner(
                *options->output_path,
                *options->matlab_plot_path);
        }
        return kSuccess;
    } catch (const std::exception& error) {
        return report_runtime_error(standard_err, error.what());
    }
}

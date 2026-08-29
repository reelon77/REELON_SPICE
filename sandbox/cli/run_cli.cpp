#include "run_cli.h"

#include "output/result_writer.h"
#include "parser/Circuit.h"
#include "sim/simulate.h"

#include <exception>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>

namespace {
constexpr int kSuccess = 0;
constexpr int kRuntimeFailure = 1;
constexpr int kUsageFailure = 2;
constexpr const char* kUsage =
    "usage: TinySpice <netlist-file> [-o <output-file>]\n";

bool has_valid_syntax(const std::vector<std::string>& args) {
    if (args.size() == 1) {
        return !args[0].empty();
    }
    return args.size() == 3
        && !args[0].empty()
        && args[1] == "-o"
        && !args[2].empty();
}

int report_usage_error(std::ostream& error) {
    error << "error: invalid arguments\n" << kUsage;
    return kUsageFailure;
}

int report_runtime_error(std::ostream& error, const std::string& message) {
    error << "error: " << message << '\n';
    return kRuntimeFailure;
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
    if (!has_valid_syntax(args)) {
        return report_usage_error(standard_err);
    }

    const std::string& input_path = args[0];
    const bool writes_file = args.size() == 3;
    if (writes_file && args[2] == input_path) {
        return report_runtime_error(
            standard_err,
            "output file must differ from input file '" + input_path + "'");
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

        if (writes_file) {
            write_output_file(args[2], circuit, result);
        } else {
            write_simulation_result(standard_out, circuit, result);
        }
        return kSuccess;
    } catch (const std::exception& error) {
        return report_runtime_error(standard_err, error.what());
    }
}

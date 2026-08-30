#include "result_writer.h"
#include "parser/Circuit.h"

#include <cstddef>
#include <iomanip>
#include <limits>
#include <locale>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <variant>

namespace {
std::size_t validate_metadata_and_get_dimension(const Circuit& circuit) {
    if (circuit.nodes < 1) {
        throw std::invalid_argument("Circuit must contain the ground node");
    }
    if (circuit.num_branch_unknowns < 0) {
        throw std::invalid_argument("Circuit branch count cannot be negative");
    }
    if (circuit.node_names.size() != static_cast<std::size_t>(circuit.nodes)) {
        throw std::invalid_argument("Circuit node metadata is inconsistent");
    }
    if (circuit.node_names.empty() || circuit.node_names[0] != "0") {
        throw std::invalid_argument("Circuit ground name must be 0");
    }
    if (circuit.branch_names.size()
        != static_cast<std::size_t>(circuit.num_branch_unknowns)) {
        throw std::invalid_argument("Circuit branch metadata is inconsistent");
    }
    return circuit.node_names.size() - 1 + circuit.branch_names.size();
}

void validate_solution_dimension(
    const std::vector<double>& x,
    std::size_t expected_dimension) {
    if (x.size() != expected_dimension) {
        throw std::invalid_argument(
            "Simulation result dimension does not match Circuit metadata");
    }
}

void validate_csv_name(const std::string& name) {
    if (name.find_first_of(",\"\r\n") != std::string::npos) {
        throw std::invalid_argument(
            "CSV labels cannot contain comma, quote, CR, or LF");
    }
}

void validate_csv_names(const Circuit& circuit) {
    for (const std::string& name : circuit.node_names) {
        validate_csv_name(name);
    }
    for (const std::string& name : circuit.branch_names) {
        validate_csv_name(name);
    }
}

void ensure_stream_ready(const std::ostream& out) {
    if (out.rdbuf() == nullptr || !out) {
        throw std::runtime_error("Output stream is not writable");
    }
}

void ensure_write_succeeded(const std::ostream& out) {
    if (!out) {
        throw std::runtime_error("Failed to write simulation result");
    }
}

void configure_numeric_format(std::ostream& out) {
    out.imbue(std::locale::classic());
    out << std::defaultfloat
        << std::setprecision(std::numeric_limits<double>::max_digits10)
        << std::noshowpoint
        << std::noshowpos
        << std::nouppercase
        << std::dec;
}

void write_solution_csv_header(
    std::ostream& out,
    const Circuit& circuit) {
    for (std::size_t i = 1; i < circuit.node_names.size(); ++i) {
        out << ",V(" << circuit.node_names[i] << ')';
    }
    for (const std::string& branch_name : circuit.branch_names) {
        out << ",I(" << branch_name << ')';
    }
}
} // namespace

void write_operating_point(
    std::ostream& out,
    const Circuit& circuit,
    const OperatingPointResult& result) {
    const std::size_t expected_dimension =
        validate_metadata_and_get_dimension(circuit);
    validate_solution_dimension(result.x, expected_dimension);
    ensure_stream_ready(out);

    std::ostream formatted(out.rdbuf());
    configure_numeric_format(formatted);

    formatted << "analysis: .op\n";
    for (std::size_t i = 1; i < circuit.node_names.size(); ++i) {
        formatted << "V(" << circuit.node_names[i] << ") = "
                  << result.x[i - 1] << '\n';
    }

    const std::size_t branch_offset = circuit.node_names.size() - 1;
    for (std::size_t i = 0; i < circuit.branch_names.size(); ++i) {
        formatted << "I(" << circuit.branch_names[i] << ") = "
                  << result.x[branch_offset + i] << '\n';
    }
    formatted << "newton_iterations = " << result.iterations << '\n';

    ensure_write_succeeded(formatted);
}

void write_transient_csv(
    std::ostream& out,
    const Circuit& circuit,
    const TransientAnalysisResult& result) {
    const std::size_t expected_dimension =
        validate_metadata_and_get_dimension(circuit);
    if (result.trajectory.empty()) {
        throw std::invalid_argument(
            "Transient result must contain at least one point");
    }
    for (const TransientPoint& point : result.trajectory) {
        validate_solution_dimension(point.x, expected_dimension);
    }
    validate_csv_names(circuit);
    ensure_stream_ready(out);

    std::ostream formatted(out.rdbuf());
    configure_numeric_format(formatted);

    formatted << "time";
    write_solution_csv_header(formatted, circuit);
    formatted << '\n';

    for (const TransientPoint& point : result.trajectory) {
        formatted << point.time;
        for (double value : point.x) {
            formatted << ',' << value;
        }
        formatted << '\n';
    }

    ensure_write_succeeded(formatted);
}

void write_dc_sweep_csv(
    std::ostream& out,
    const Circuit& circuit,
    const DcSweepAnalysisResult& result) {
    const std::size_t expected_dimension =
        validate_metadata_and_get_dimension(circuit);
    if (result.source_names.size() != 1
        && result.source_names.size() != 2) {
        throw std::invalid_argument(
            "DC sweep result must contain one or two source names");
    }
    std::unordered_set<std::string> unique_source_names;
    for (const std::string& source_name : result.source_names) {
        if (source_name.empty()) {
            throw std::invalid_argument(
                "DC sweep source names cannot be empty");
        }
        validate_csv_name(source_name);
        if (!unique_source_names.insert(source_name).second) {
            throw std::invalid_argument(
                "DC sweep source names must be unique");
        }
    }
    if (result.points.empty()) {
        throw std::invalid_argument(
            "DC sweep result must contain at least one point");
    }
    for (const DcSweepPoint& point : result.points) {
        if (point.source_values.size() != result.source_names.size()) {
            throw std::invalid_argument(
                "DC sweep point values do not match source names");
        }
        validate_solution_dimension(point.x, expected_dimension);
    }
    validate_csv_names(circuit);
    ensure_stream_ready(out);

    std::ostream formatted(out.rdbuf());
    configure_numeric_format(formatted);

    for (std::size_t index = 0; index < result.source_names.size(); ++index) {
        if (index != 0) {
            formatted << ',';
        }
        formatted << "sweep(" << result.source_names[index] << ')';
    }
    write_solution_csv_header(formatted, circuit);
    formatted << '\n';

    for (const DcSweepPoint& point : result.points) {
        for (std::size_t index = 0; index < point.source_values.size(); ++index) {
            if (index != 0) {
                formatted << ',';
            }
            formatted << point.source_values[index];
        }
        for (double value : point.x) {
            formatted << ',' << value;
        }
        formatted << '\n';
    }

    ensure_write_succeeded(formatted);
}

void write_simulation_result(
    std::ostream& out,
    const Circuit& circuit,
    const SimulationResult& result) {
    std::visit(
        [&](const auto& typed_result) {
            using ResultType = std::decay_t<decltype(typed_result)>;
            if constexpr (std::is_same_v<ResultType, OperatingPointResult>) {
                write_operating_point(out, circuit, typed_result);
            } else if constexpr (
                std::is_same_v<ResultType, TransientAnalysisResult>) {
                write_transient_csv(out, circuit, typed_result);
            } else {
                write_dc_sweep_csv(out, circuit, typed_result);
            }
        },
        result);
}

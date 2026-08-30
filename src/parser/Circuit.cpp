#include "Circuit.h"
#include "devices/VoltageSource.h"
#include "devices/CurrentSource.h"
#include "devices/IndependentSource.h"
#include "devices/Capacitor.h"
#include "devices/Inductor.h"
#include "devices/Diode.h"
#include "tokenize.h"
#include "numeric.h"
#include "devices/Resistor.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>
#include <memory>
#include <istream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <variant>

std::vector<double> generate_dc_sweep_values(
    double start,
    double stop,
    double step) {
    if (!std::isfinite(start)
        || !std::isfinite(stop)
        || !std::isfinite(step)) {
        throw std::invalid_argument("DC sweep values must be finite");
    }
    if (step == 0.0) {
        throw std::invalid_argument("DC sweep step cannot be zero");
    }
    if ((start < stop && step < 0.0)
        || (start > stop && step > 0.0)) {
        throw std::invalid_argument(
            "DC sweep step points away from stop");
    }
    if (start == stop) {
        return {start};
    }

    const bool ascending = step > 0.0;
    std::vector<double> values;
    for (std::size_t index = 0; ; ++index) {
        const double value =
            start + static_cast<double>(index) * step;
        if (!std::isfinite(value)) {
            throw std::overflow_error("DC sweep value overflowed");
        }
        const double scale = std::max({
            std::abs(start),
            std::abs(stop),
            std::abs(value),
        });
        const double adjacent_stop = std::nextafter(
            stop,
            ascending
                ? std::numeric_limits<double>::infinity()
                : -std::numeric_limits<double>::infinity());
        const double stop_ulp = std::abs(adjacent_stop - stop);
        const double roundoff_tolerance = std::max(
            4.0 * stop_ulp,
            8.0 * std::numeric_limits<double>::epsilon() * scale);
        const double tolerance = std::min(
            std::abs(step) / 4.0,
            roundoff_tolerance);
        const bool past_stop = ascending
            ? value > stop + tolerance
            : value < stop - tolerance;
        if (past_stop) {
            break;
        }
        if (std::abs(value - stop) <= tolerance) {
            values.push_back(stop);
            break;
        }
        if (!values.empty()) {
            const bool advances = ascending
                ? value > values.back()
                : value < values.back();
            if (!advances) {
                throw std::overflow_error(
                    "DC sweep step cannot advance value");
            }
        }
        values.push_back(value);
        if (index == std::numeric_limits<std::size_t>::max()) {
            throw std::overflow_error("DC sweep has too many points");
        }
        const double next =
            start + static_cast<double>(index + 1) * step;
        if (next == value) {
            throw std::overflow_error("DC sweep step cannot advance value");
        }
    }
    return values;
}

Circuit parse_circuit(std::istream& input) {
    Circuit circuit;
    std::string line;
    int line_number = 0;

    std::unordered_map<std::string, int> nodes_map;
    nodes_map["0"] = 0;
    nodes_map["gnd"] = 0;
    int next_node = 1;
    std::unordered_map<std::string, int> device_name_lines;
    int analysis_line = 0;
    std::string analysis_directive;

    auto get_or_create_node = [&nodes_map, &next_node, &circuit](const std::string& node_name) -> int {
        auto res = nodes_map.find(node_name);
        if (res != nodes_map.end()) {
            return res->second;
        }
        int new_node = next_node;
        if (node_name != "0" && node_name != "gnd") {
            circuit.node_names.push_back(node_name);
        }
        nodes_map.emplace(node_name, new_node);
        next_node++;
        return new_node;
    };

    auto parse_value = [&line_number](const std::string& token_s) -> double {
        try {
            return string2double(token_s);
        } catch(const std::exception& e) {
            std::stringstream err;
            err << line_number << ": " << token_s << ":" << e.what();
            throw std::runtime_error(err.str());
        }
    };

    auto add_device = [
        &circuit,
        &device_name_lines,
        &line_number](
            const std::string& name,
            std::unique_ptr<Device> device) {
        const auto [existing, inserted] =
            device_name_lines.emplace(name, line_number);
        if (!inserted) {
            std::stringstream error;
            error << line_number << ": duplicate device name " << name
                  << " (first declared at line " << existing->second << ')';
            throw std::runtime_error(error.str());
        }
        circuit.device_names.push_back(name);
        circuit.devices.push_back(std::move(device));
    };

    auto set_analysis = [
        &circuit,
        &analysis_line,
        &analysis_directive,
        &line_number](
            const std::string& directive,
            AnalysisRequest request) {
        if (!std::holds_alternative<std::monostate>(circuit.analysis)) {
            std::stringstream error;
            error << line_number << ": analysis directive " << directive
                  << " conflicts with " << analysis_directive
                  << " at line " << analysis_line;
            throw std::runtime_error(error.str());
        }
        circuit.analysis = std::move(request);
        analysis_line = line_number;
        analysis_directive = directive;
    };

    auto validate_dc_axis = [&line_number](const DcSweepAxis& axis) {
        try {
            (void)generate_dc_sweep_values(
                axis.start,
                axis.stop,
                axis.step);
        } catch (const std::exception& error) {
            std::stringstream message;
            message << line_number << ": .dc " << axis.source_name
                    << ": " << error.what();
            throw std::runtime_error(message.str());
        }
    };

    while (std::getline(input, line)) {
        ++line_number;
        auto tokens = tokenize(line);

        if (tokens.empty()) {
            continue;
        }
        
        if (tokens[0] == ".op") {
            if (tokens.size() != 1) {
                std::stringstream e;
                e << line_number << ": .op line's more than 1 token!";
                throw std::runtime_error(e.str());
            }
            set_analysis(tokens[0], OperatingPointAnalysis{});
            continue;
        }

        if (tokens[0] == ".end") {
            if (tokens.size() != 1) {
                std::stringstream e;
                e << line_number << ": .end line's more than 1 token!";
                throw std::runtime_error(e.str());
            }
            break;
        }

        if (tokens[0] == ".dc") {
            if (tokens.size() != 5 && tokens.size() != 9) {
                std::stringstream error;
                error << line_number
                      << ": .dc requires 5 or 9 tokens";
                throw std::runtime_error(error.str());
            }
            constexpr std::size_t unresolved_index =
                std::numeric_limits<std::size_t>::max();
            DcSweepAxis primary{
                tokens[1],
                unresolved_index,
                parse_value(tokens[2]),
                parse_value(tokens[3]),
                parse_value(tokens[4]),
            };
            validate_dc_axis(primary);

            std::optional<DcSweepAxis> secondary;
            if (tokens.size() == 9) {
                secondary = DcSweepAxis{
                    tokens[5],
                    unresolved_index,
                    parse_value(tokens[6]),
                    parse_value(tokens[7]),
                    parse_value(tokens[8]),
                };
                validate_dc_axis(*secondary);
                if (primary.source_name == secondary->source_name) {
                    std::stringstream error;
                    error << line_number
                          << ": .dc sources must be different: "
                          << primary.source_name;
                    throw std::runtime_error(error.str());
                }
            }
            set_analysis(
                tokens[0],
                DcSweepAnalysis{
                    std::move(primary),
                    std::move(secondary),
                });
            continue;
        }

        if (tokens[0][0] == 'r') {
            if (tokens.size() == 4) {
                double resistance = parse_value(tokens[3]);
                int node1 = get_or_create_node(tokens[1]);
                int node2 = get_or_create_node(tokens[2]);
                add_device(
                    tokens[0],
                    std::make_unique<Resistor>(resistance, node1, node2));
                continue;
            } else {
                std::stringstream e;
                e << line_number << ": " << "the number of tokens is incorrect!";
                throw std::runtime_error(e.str());
            }
        }
        if (tokens[0][0] == 'v') {
            if (tokens.size() == 4) {
                double voltage = parse_value(tokens[3]);
                int node_pos = get_or_create_node(tokens[1]);
                int node_neg = get_or_create_node(tokens[2]);
                int sourceIndex = circuit.num_branch_unknowns;
                add_device(
                    tokens[0],
                    std::make_unique<VoltageSource>(
                        voltage,
                        node_pos,
                        node_neg,
                        sourceIndex));
                circuit.branch_names.push_back(tokens[0]);
                circuit.num_branch_unknowns++;
                continue;
            } else {
                std::stringstream e;
                e << line_number << ": " << "the number of tokens is incorrect!";
                throw std::runtime_error(e.str());
            }
        }
        if (tokens[0][0] == 'i') {
            if (tokens.size() == 4) {
                double current = parse_value(tokens[3]);
                int node_from = get_or_create_node(tokens[1]);
                int node_to = get_or_create_node(tokens[2]);
                add_device(
                    tokens[0],
                    std::make_unique<CurrentSource>(
                        current,
                        node_from,
                        node_to));
                continue;
            } else {
                std::stringstream e;
                e << line_number << ": " << "the number of tokens is incorrect!";
                throw std::runtime_error(e.str());
            }
        }
        if (tokens[0][0] == 'd') {
            if (tokens.size() == 3) {
                double Is = 1e-12;
                double Vt = 0.025852;
                int node_pos = get_or_create_node(tokens[1]);
                int node_neg = get_or_create_node(tokens[2]);
                add_device(
                    tokens[0],
                    std::make_unique<Diode>(Is, Vt, node_pos, node_neg));
                continue;
            } else {
                std::stringstream e;
                e << line_number << ": " << "the number of tokens is incorrect!";
                throw std::runtime_error(e.str());
            }
        }
        if (tokens[0][0] == 'c') {
            if (tokens.size() != 4) {
                std::stringstream e;
                e << line_number << ": The number of parameters must be equal to 4!";
                throw std::runtime_error(e.str());
            }
            double capacity = parse_value(tokens[3]);
            if (capacity <= 0) {
                std::stringstream e;
                e << line_number << ": The capacity value must be positive!";
                throw std::runtime_error(e.str());
            }
            int node_pos = get_or_create_node(tokens[1]);
            int node_neg = get_or_create_node(tokens[2]);
            add_device(
                tokens[0],
                std::make_unique<Capacitor>(capacity, node_pos, node_neg));
            continue;
        }
        if (tokens[0][0] == 'l') {
            if (tokens.size() != 4) {
                std::stringstream e;
                e << line_number << ": The number of parameters must be equal to 4!";
                throw std::runtime_error(e.str());
            }
            double L = parse_value(tokens[3]);
            if (L <= 0) {
                std::stringstream e;
                e << line_number << ": The L value must be positive!";
                throw std::runtime_error(e.str());
            }
            int node_pos = get_or_create_node(tokens[1]);
            int node_neg = get_or_create_node(tokens[2]);
            add_device(
                tokens[0],
                std::make_unique<Inductor>(
                    L,
                    node_pos,
                    node_neg,
                    circuit.num_branch_unknowns));
            circuit.branch_names.push_back(tokens[0]);
            circuit.num_branch_unknowns++;
            continue;
        }
        if (tokens[0] == ".tran") {
            if (tokens.size() != 3) {
                std::stringstream e;
                e << line_number << ": The parameters in .tran command must be equal to 3!";
                throw std::runtime_error(e.str());
            }
            double t_step = parse_value(tokens[1]);
            double t_stop = parse_value(tokens[2]);
            if (t_step <= 0 || t_stop <= 0) {
                std::stringstream e;
                e << line_number << ": The t_step and t_stop value must be positive!";
                throw std::runtime_error(e.str());
            }
            set_analysis(tokens[0], TransientAnalysis{t_step, t_stop});
            continue;
        }

        std::stringstream e;
        e << line_number << ": undefined token " << tokens[0];
        throw std::runtime_error(e.str());
    }
    circuit.nodes = next_node;

    if (auto* dc = std::get_if<DcSweepAnalysis>(&circuit.analysis)) {
        auto resolve_source = [
            &circuit,
            analysis_line](DcSweepAxis& axis) {
            const auto found = std::find(
                circuit.device_names.begin(),
                circuit.device_names.end(),
                axis.source_name);
            if (found == circuit.device_names.end()) {
                std::stringstream error;
                error << analysis_line << ": .dc source "
                      << axis.source_name << " does not exist";
                throw std::runtime_error(error.str());
            }
            axis.device_index = static_cast<std::size_t>(
                std::distance(circuit.device_names.begin(), found));
            if (dynamic_cast<IndependentSource*>(
                    circuit.devices[axis.device_index].get()) == nullptr) {
                std::stringstream error;
                error << analysis_line << ": .dc source "
                      << axis.source_name
                      << " is not an independent voltage or current source";
                throw std::runtime_error(error.str());
            }
        };
        resolve_source(dc->primary);
        if (dc->secondary) {
            resolve_source(*dc->secondary);
        }
    }
    return circuit;
}

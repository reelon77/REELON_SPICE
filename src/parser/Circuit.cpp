#include "Circuit.h"
#include "devices/VoltageSource.h"
#include "devices/CurrentSource.h"
#include "devices/Capacitor.h"
#include "devices/Inductor.h"
#include "devices/Diode.h"
#include "tokenize.h"
#include "numeric.h"
#include "devices/Resistor.h"

#include <exception>
#include <memory>
#include <istream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <unordered_map>

Circuit parse_circuit(std::istream& input) {
    Circuit circuit;
    std::string line;
    int line_number = 0;

    std::unordered_map<std::string, int> nodes_map;
    nodes_map["0"] = 0;
    nodes_map["gnd"] = 0;
    int next_node = 1;

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
            circuit.analysis_type = AnalysisType::Op;
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

        if (tokens[0][0] == 'r') {
            if (tokens.size() == 4) {
                double resistance = parse_value(tokens[3]);
                int node1 = get_or_create_node(tokens[1]);
                int node2 = get_or_create_node(tokens[2]);
                circuit.devices.push_back(std::make_unique<Resistor>(resistance, node1, node2));
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
                circuit.devices.push_back(std::make_unique<VoltageSource>(voltage, node_pos, node_neg, sourceIndex));
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
                circuit.devices.push_back(std::make_unique<CurrentSource>(current, node_from, node_to));
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
                circuit.devices.push_back(std::make_unique<Diode>(Is, Vt, node_pos, node_neg));
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
            circuit.devices.push_back(std::make_unique<Capacitor>(capacity, node_pos, node_neg));
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
            circuit.devices.push_back(std::make_unique<Inductor>(L, node_pos, node_neg, circuit.num_branch_unknowns));
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
            circuit.t_step = t_step;
            circuit.t_stop = t_stop;
            circuit.analysis_type = AnalysisType::Tran;
            continue;
        }

        std::stringstream e;
        e << line_number << ": undefined token " << tokens[0];
        throw std::runtime_error(e.str());
    }
    circuit.nodes = next_node;
    return circuit;
}

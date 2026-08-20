#include "Circuit.h"
#include "tokenize.h"

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

    auto get_or_create_node = [&nodes_map, &next_node](const std::string& node_name) -> int {
        auto res = nodes_map.find(node_name);
        if (res != nodes_map.end()) {
            return res->second;
        }
        int new_node = next_node;
        nodes_map.emplace(node_name, new_node);
        next_node++;
        return new_node;
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

        std::string legal_devices = "rvid";
        if (legal_devices.find(tokens[0][0]) != std::string::npos && tokens.size() >= 3) {
            get_or_create_node(tokens[1]);
            get_or_create_node(tokens[2]);
            continue;
        }
    }
    circuit.nodes = next_node;
    return circuit;
}

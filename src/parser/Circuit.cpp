#include "Circuit.h"
#include "tokenize.h"

#include <istream>
#include <sstream>
#include <string>
#include <stdexcept>

Circuit parse_circuit(std::istream& input) {
    Circuit circuit;
    std::string line;
    int line_number = 0;
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
    }

    return circuit;
}

#include "parser/numeric.h"
#include <stdexcept>

double string2double(const std::string& str) {
    size_t pos;
    double base = std::stod(str, &pos);
    if (pos < str.length()) {
        std::string unit = str.substr(pos);
        if (unit == "k") {
            base *= 1e3;
        }
        else if (unit == "g") {
            base *= 1e9;
        }
        else if (unit == "t") {
            base *= 1e12;
        }
        else if (unit == "m") {
            base *= 1e-3;
        } else if (unit == "u") {
            base *= 1e-6;
        } else if (unit == "n") {
            base *= 1e-9;
        } else if (unit == "p") {
            base *= 1e-12;
        } else if (unit == "f") {
            base *= 1e-15;
        } else if (unit == "meg") {
            base *= 1e6;
        } else {
            throw std::invalid_argument("Invalid unit format: " + unit);
        }
    }
    return base;
}
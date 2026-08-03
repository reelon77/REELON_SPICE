#include "Resistor.h"
#include "mna/MnaSystem.h"
#include <stdexcept>

namespace {
    double conductance_from(double r) {
        if (r <= 0) {throw std::invalid_argument("Resistance must be positive.");}
        return 1.0 / r;
    }
}

Resistor::Resistor(double resistance, int node1, int node2)
    : g_(conductance_from(resistance)), node1_(node1), node2_(node2) {
    }

void Resistor::stamp(MnaSystem& mna) const {
    // Implementation for stamping resistor in MNA system
    mna.add_to_A(node1_, node2_, -g_);
    mna.add_to_A(node2_, node1_, -g_);
    mna.add_to_A(node1_, node1_, g_);
    mna.add_to_A(node2_, node2_, g_);
}

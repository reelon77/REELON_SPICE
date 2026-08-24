#include "Capacitor.h"
#include "mna/MnaSystem.h"

#include <stdexcept>
#include <sstream>

Capacitor::Capacitor(double capacity, int node_pos, int node_neg) : capacity_(capacity), node_neg_(node_neg), node_pos_(node_pos) {
    if (capacity_ <= 0) {
        std::stringstream err;
        err << "capacity value can not be negative!";
        throw std::invalid_argument(err.str());
    }
}

void Capacitor::stamp(MnaSystem&) const {
}

void Capacitor::stamp(MnaSystem& mna, const std::vector<double>& x, const TransientContext& ctx) const {
    if (ctx.dt <= 0) {
        std::stringstream e;
        e << "dt must be positive!";
        throw std::invalid_argument(e.str());
    }

    double voltage_pos_prev = 0;
    double voltage_neg_prev = 0;
    if (node_pos_ != 0) {
        voltage_pos_prev = ctx.x_prev[node_pos_ - 1];
    }
    if (node_neg_ != 0) {
        voltage_neg_prev = ctx.x_prev[node_neg_ - 1];
    }
    double voltage_prev = voltage_pos_prev - voltage_neg_prev;

    double G_eq = capacity_ / ctx.dt;
    double I_hist = -G_eq * voltage_prev;
    mna.add_to_A(node_pos_, node_pos_, G_eq);
    mna.add_to_A(node_neg_, node_neg_, G_eq);
    mna.add_to_A(node_neg_, node_pos_, -G_eq);
    mna.add_to_A(node_pos_, node_neg_, -G_eq);
    mna.add_to_b(node_pos_, -I_hist);
    mna.add_to_b(node_neg_, I_hist);
}
#include <stdexcept>
#include <cmath>

#include "mna/MnaSystem.h"
#include "Diode.h"

Diode::Diode(double Is, double Vt, int node_pos, int node_neg)
    : Is_(Is), Vt_(Vt), node_pos_(node_pos), node_neg_(node_neg) {}

void Diode::stamp(MnaSystem& mna) const {
    throw std::logic_error("Diode::stamp needs the current iterate; use stamp(mna, x)");
}

void Diode::stamp(MnaSystem& mna, const std::vector<double>& x) const {
    double v;
    if (node_neg_ == 0) {
        v = x[node_pos_ - 1];
    } else if (node_pos_ == 0) {
        v = -x[node_neg_ - 1];
    } else {
        v = x[node_pos_ - 1] - x[node_neg_ - 1];
    }
    double I;
    double G_eq;
    {
        double tmp = std::exp(v / Vt_);
        I = Is_ * (tmp - 1);
        G_eq = Is_ / Vt_ * tmp;
    }
    double I_eq = I - G_eq * v;

    mna.add_to_A(node_pos_, node_pos_, G_eq);
    mna.add_to_A(node_pos_, node_neg_, -G_eq);
    mna.add_to_A(node_neg_, node_pos_, -G_eq);
    mna.add_to_A(node_neg_, node_neg_, G_eq);
    mna.add_to_b(node_pos_, -I_eq);
    mna.add_to_b(node_neg_, I_eq);
}
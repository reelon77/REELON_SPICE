#include "Inductor.h"
#include "mna/MnaSystem.h"

#include <stdexcept>
#include <sstream>

Inductor::Inductor(double inductance, int node_pos, int node_neg, int branchIndex) 
    : inductance_(inductance), node_pos_(node_pos), node_neg_(node_neg), branchIndex_(branchIndex){
        if (inductance <= 0) {
            std::stringstream e;
            e << "The inductance value must be positive!";
            throw std::invalid_argument(e.str());
        }
}

void Inductor::stamp(MnaSystem& mna) const {
    int k = mna.branch_index(branchIndex_);
    if (node_pos_ != 0) {
        mna.add_to_A_raw(node_pos_ - 1, k, 1.0);
        mna.add_to_A_raw(k, node_pos_ - 1, 1.0);
    }
    if (node_neg_ != 0) {
        mna.add_to_A_raw(node_neg_ - 1, k, -1.0);
        mna.add_to_A_raw(k, node_neg_ - 1, -1.0);
    }
}

void Inductor::stamp(MnaSystem& mna, const std::vector<double>& x, const TransientContext& ctx) const {
    if (ctx.dt <= 0) {
        std::stringstream e;
        e << "The dt value must be positive!";
        throw std::invalid_argument(e.str());
    }
    int k = mna.branch_index(branchIndex_);
    double I_prev = ctx.x_prev[k];
    double R_eq = inductance_ / ctx.dt;

    stamp(mna);
    mna.add_to_A_raw(k, k, -R_eq);
    mna.add_to_b_raw(k, -R_eq * I_prev);
}


#include "MnaSystem.h"
#include <algorithm>

MnaSystem::MnaSystem(int nodes, int voltageSources) : 
    A_(nodes - 1 + voltageSources),
    b_(nodes - 1 + voltageSources, 0.0),
    nodes_voltages_(nodes - 1),
    voltageSources_(voltageSources) 
    {}

void MnaSystem::add_to_A(int node_i, int node_j, double val) {
    if (node_i == 0 || node_j == 0) {
        return;
    }
    add_to_A_raw(node_i - 1, node_j - 1, val);
}

void MnaSystem::add_to_b(int node_i, double val) {
    if (node_i == 0) {
        return;
    }
    add_to_b_raw(node_i - 1, val);
}

int MnaSystem::branch_index(int voltageSource_i) const {
    return nodes_voltages_ + voltageSource_i;
}

void MnaSystem::add_to_A_raw(int row, int col, double val) {
    A_(row, col) += val;
}

void MnaSystem::add_to_b_raw(int row, double val) {
    b_[row] += val;
}

const Matrix& MnaSystem::get_A() const {
    return A_;
}

const std::vector<double>& MnaSystem::get_b() const {
    return b_;
}

void MnaSystem::clear() {
    A_.zeros();
    std::fill(b_.begin(), b_.end(), 0.0);
}

int MnaSystem::dim() const {
    return nodes_voltages_ + voltageSources_;
}
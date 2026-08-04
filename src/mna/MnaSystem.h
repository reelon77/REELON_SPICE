#pragma once

#include "core/Matrix.h"
#include <vector>

class MnaSystem {
public:
    // 构造函数的nodes包含了地节点
    MnaSystem(int nodes, int voltageSources);
    void add_to_A(int node_i, int node_j, double val);
    void add_to_b(int node_i, double val);
    int branch_index(int voltageSource_i) const;
    void add_to_A_raw(int row, int col, double val);
    void add_to_b_raw(int row, double val);
    const Matrix& get_A() const;
    const std::vector<double>& get_b() const;
    void clear();
    int dim() const;
private:
    Matrix A_;
    std::vector<double> b_;
    int nodes_voltages_;
    int voltageSources_;
};
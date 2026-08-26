#pragma once

#include <memory>
#include <vector>
#include <iosfwd>

#include "../devices/Device.h"

enum class AnalysisType {
    None,
    Tran,
    Op,
};

struct Circuit {
    std::vector<std::unique_ptr<Device>> devices;
    int nodes = 1;  // 包含地节点
    int num_branch_unknowns = 0;    // 每个 V/L 各占一个支路未知量，按网表出现顺序共同编号
    AnalysisType analysis_type = AnalysisType::None;
    double t_step = 0.0;
    double t_stop = 0.0;
};

Circuit parse_circuit(std::istream& input);

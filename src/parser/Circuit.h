#pragma once

#include <memory>
#include <optional>
#include <cstddef>
#include <vector>
#include <iosfwd>
#include <string>
#include <variant>

#include "../devices/Device.h"

struct OperatingPointAnalysis {};

struct TransientAnalysis {
    double t_step;
    double t_stop;
};

struct DcSweepAxis {
    std::string source_name;
    std::size_t device_index;
    double start;
    double stop;
    double step;
};

struct DcSweepAnalysis {
    DcSweepAxis primary;
    std::optional<DcSweepAxis> secondary;
};

using AnalysisRequest = std::variant<
    std::monostate,
    OperatingPointAnalysis,
    TransientAnalysis,
    DcSweepAnalysis>;

struct Circuit {
    std::vector<std::string> node_names{"0"};
    std::vector<std::string> branch_names;
    std::vector<std::string> device_names;
    std::vector<std::unique_ptr<Device>> devices;
    int nodes = 1;  // 包含地节点
    int num_branch_unknowns = 0;    // 每个 V/L 各占一个支路未知量，按网表出现顺序共同编号
    AnalysisRequest analysis;
};

Circuit parse_circuit(std::istream& input);

std::vector<double> generate_dc_sweep_values(
    double start,
    double stop,
    double step);

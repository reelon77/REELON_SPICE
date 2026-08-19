#pragma once

#include <memory>
#include <vector>
#include <iosfwd>

#include "../devices/Device.h"

enum class AnalysisType {
    None,
    Op,
};

struct Circuit {
    std::vector<std::unique_ptr<Device>> devices;
    int nodes = 1;  // 包含地节点
    int num_voltage_sources = 0;    // 电压源编号由解析顺序决定，按网表中 V 器件的出现顺序、从 0 开始
    AnalysisType analysis_type = AnalysisType::None;
};

Circuit parse_circuit(std::istream& input);

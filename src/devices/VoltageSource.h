#pragma once

#include "Device.h"

// 独立电压源。
//
// 极性约定：node_pos 是正端、node_neg 是负端，支路方程为
//     v(node_pos) − v(node_neg) = voltage
// 支路电流未知量 i 的正方向：在源内部从 node_pos 流向 node_neg
// （所以给电路供电时 i < 0，见端到端测试里 i_V1 = −2mA 的断言）。
class VoltageSource : public Device {
public:
    VoltageSource(double voltage, int node_pos, int node_neg, int sourceIndex);
    void stamp(MnaSystem& mna) const override;
private:
    double voltage_;
    int node_pos_;
    int node_neg_;
    int sourceIndex_;
};

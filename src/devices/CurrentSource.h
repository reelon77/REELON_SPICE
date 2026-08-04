#pragma once

#include "Device.h"

// 独立电流源。
//
// 方向约定：电流 current 在源内部从 node_from 流向 node_to，
// 即对外部电路而言：从 node_from 抽走 current，注入 node_to。
// KCL 右端项因此是 b[node_from] −= current、b[node_to] += current。
class CurrentSource : public Device {
public:
    CurrentSource(double current, int node_from, int node_to);
    void stamp(MnaSystem& mna) const override;
private:
    double current_;
    int node_from_;
    int node_to_;
};

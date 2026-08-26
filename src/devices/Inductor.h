#pragma once

#include "devices/Device.h"

class Inductor : public Device {
public:
    Inductor(double inductance, int node_pos, int node_neg, int branchIndex);
    virtual void stamp(MnaSystem& mna) const override;
    virtual void stamp(MnaSystem& mna, const std::vector<double>& x, const TransientContext& ctx) const override;
private:
    double inductance_;
    int node_pos_;
    int node_neg_;
    int branchIndex_;
};
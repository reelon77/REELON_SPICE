#pragma once

#include "Device.h"

class Capacitor : public Device {
public:
    Capacitor(double capacity, int node_pos, int node_neg);
    virtual void stamp(MnaSystem&) const override;
    virtual void stamp(
        MnaSystem& mna, 
        const std::vector<double>& x, 
        const TransientContext& ctx) const override;
private:
    double capacity_;
    int node_pos_;
    int node_neg_;
};
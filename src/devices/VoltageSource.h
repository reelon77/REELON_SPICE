#pragma once

#include "Device.h"

class VoltageSource : public Device {
public:
    VoltageSource(double voltage, int node1, int node2, int sourceIndex);
    void stamp(MnaSystem& mna) const override;
private:
    double voltage_;
    int node1_;
    int node2_;
    int sourceIndex_;
};
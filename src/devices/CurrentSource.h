#pragma once

#include "Device.h"

class CurrentSource : public Device {
public:
    CurrentSource(double current, int node1, int node2);
    void stamp(MnaSystem& mna) const override;
private:
    double current_;
    int node1_;
    int node2_;
};
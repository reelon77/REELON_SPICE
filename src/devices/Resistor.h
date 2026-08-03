#pragma once

#include "Device.h"

class Resistor : public Device {
public:
    Resistor(double resistance, int node1, int node2);
    void stamp(MnaSystem& mna) const override;
private:
    double g_;
    int node1_;
    int node2_;
};
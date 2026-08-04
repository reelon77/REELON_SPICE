#pragma once

#include "Device.h"
#include <vector>

class Diode : public Device {
public:
    Diode(double Is, double Vt, int node_pos, int node_neg);
    void stamp(MnaSystem& mna) const override;
    void stamp(MnaSystem& mna, const std::vector<double>& x) const override;
private:
    double Is_; // Saturation current
    double Vt_; // Thermal voltage
    // I = Is·(e^(v/Vt) − 1),其中 v ≡ v(node_pos) − v(node_neg),正电流方向 pos→neg(阳极→阴极)
    int node_pos_;
    int node_neg_;
};
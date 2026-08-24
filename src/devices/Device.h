#pragma once

#include <vector>

class MnaSystem;

struct TransientContext {
    double dt;
    const std::vector<double>& x_prev;
    double time;
};

class Device {
public:
    virtual void stamp(MnaSystem& mna) const = 0;
    virtual void stamp(MnaSystem& mna, const std::vector<double>& x) const;
    virtual void stamp(MnaSystem& mna, const std::vector<double>& x, const TransientContext& ctx) const;
    virtual ~Device() = default;
};
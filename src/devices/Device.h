#pragma once

#include <vector>

class MnaSystem;

class Device {
public:
    virtual void stamp(MnaSystem& mna) const = 0;
    virtual void stamp(MnaSystem& mna, const std::vector<double>& x) const;
    virtual ~Device() = default;
};
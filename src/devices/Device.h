#pragma once

class MnaSystem;

class Device {
public:
    virtual void stamp(MnaSystem& mna) const = 0;
    virtual ~Device() = default;
};
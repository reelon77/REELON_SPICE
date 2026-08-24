#include "Device.h"

void Device::stamp(MnaSystem& mna, const std::vector<double>& x) const {
    return stamp(mna);
}

void Device::stamp(MnaSystem& mna, const std::vector<double>& x, const TransientContext& ctx) const {
    return stamp(mna, x);
}

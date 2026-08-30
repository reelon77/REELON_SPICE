#pragma once

#include "Device.h"

#include <memory>

// Common interface for independent V/I sources whose DC value can be
// overridden by an analysis without mutating the owning Circuit.
class IndependentSource : public Device {
public:
    virtual double dc_value() const noexcept = 0;
    virtual std::unique_ptr<IndependentSource> clone_with_dc_value(
        double value) const = 0;
};

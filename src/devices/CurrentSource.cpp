#include "CurrentSource.h"
#include "mna/MnaSystem.h"

CurrentSource::CurrentSource(double current, int node_from, int node_to)
    : current_(current), node_from_(node_from), node_to_(node_to) {
}

void CurrentSource::stamp(MnaSystem& mna) const {
    mna.add_to_b(node_from_, -current_);
    mna.add_to_b(node_to_, current_);
}

double CurrentSource::dc_value() const noexcept {
    return current_;
}

std::unique_ptr<IndependentSource> CurrentSource::clone_with_dc_value(
    double value) const {
    return std::make_unique<CurrentSource>(value, node_from_, node_to_);
}

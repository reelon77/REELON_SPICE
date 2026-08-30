#include "VoltageSource.h"
#include "mna/MnaSystem.h"

VoltageSource::VoltageSource(double voltage, int node_pos, int node_neg, int sourceIndex)
    : voltage_(voltage), node_pos_(node_pos), node_neg_(node_neg), sourceIndex_(sourceIndex) {
}

void VoltageSource::stamp(MnaSystem& mna) const {
    int k = mna.branch_index(sourceIndex_);
    if (node_pos_ != 0) {
        mna.add_to_A_raw(node_pos_ - 1, k, 1.0);
        mna.add_to_A_raw(k, node_pos_ - 1, 1.0);
    }
    if (node_neg_ != 0) {
        mna.add_to_A_raw(node_neg_ - 1, k, -1.0);
        mna.add_to_A_raw(k, node_neg_ - 1, -1.0);
    }
    mna.add_to_b_raw(k, voltage_);

}

double VoltageSource::dc_value() const noexcept {
    return voltage_;
}

std::unique_ptr<IndependentSource> VoltageSource::clone_with_dc_value(
    double value) const {
    return std::make_unique<VoltageSource>(
        value,
        node_pos_,
        node_neg_,
        sourceIndex_);
}

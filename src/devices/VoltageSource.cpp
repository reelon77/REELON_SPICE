#include "VoltageSource.h"
#include "mna/MnaSystem.h"

VoltageSource::VoltageSource(double voltage, int node1, int node2, int sourceIndex)
    : voltage_(voltage), node1_(node1), node2_(node2), sourceIndex_(sourceIndex) {
}

void VoltageSource::stamp(MnaSystem& mna) const {
    int k = mna.branch_index(sourceIndex_);
    if (node1_ != 0) {
        mna.add_to_A_raw(node1_ - 1, k, 1.0);
        mna.add_to_A_raw(k, node1_ - 1, 1.0);
    }
    if (node2_ != 0) {
        mna.add_to_A_raw(node2_ - 1, k, -1.0);
        mna.add_to_A_raw(k, node2_ - 1, -1.0);
    }
    mna.add_to_b_raw(k, voltage_);

}
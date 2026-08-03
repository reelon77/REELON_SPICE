#include "CurrentSource.h"
#include "mna/MnaSystem.h"

CurrentSource::CurrentSource(double current, int node1, int node2)
    : current_(current), node1_(node1), node2_(node2) {
}

void CurrentSource::stamp(MnaSystem& mna) const {
    mna.add_to_b(node1_, -current_);
    mna.add_to_b(node2_, current_);
}
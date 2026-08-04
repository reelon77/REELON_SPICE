#include "CurrentSource.h"
#include "mna/MnaSystem.h"

CurrentSource::CurrentSource(double current, int node_from, int node_to)
    : current_(current), node_from_(node_from), node_to_(node_to) {
}

void CurrentSource::stamp(MnaSystem& mna) const {
    mna.add_to_b(node_from_, -current_);
    mna.add_to_b(node_to_, current_);
}

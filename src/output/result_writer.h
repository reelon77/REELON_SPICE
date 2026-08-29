#pragma once

#include "sim/simulate.h"
#include <iosfwd>

void write_operating_point(
    std::ostream& out,
    const Circuit& circuit,
    const OperatingPointResult& result);

void write_transient_csv(
    std::ostream& out,
    const Circuit& circuit,
    const TransientAnalysisResult& result);

void write_simulation_result(
    std::ostream& out,
    const Circuit& circuit,
    const SimulationResult& result);
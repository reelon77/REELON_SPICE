#pragma once

#include "solver/transient.h"

#include <variant>
#include <vector>

struct Circuit;

struct OperatingPointResult {
    std::vector<double> x;
    int iterations;
};

struct TransientAnalysisResult {
    std::vector<TransientPoint> trajectory;
};

using SimulationResult =
    std::variant<OperatingPointResult,
                 TransientAnalysisResult>;

SimulationResult simulate(const Circuit& circuit);

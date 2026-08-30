#pragma once

#include "solver/transient.h"

#include <variant>
#include <string>
#include <vector>

struct Circuit;

struct OperatingPointResult {
    std::vector<double> x;
    int iterations;
};

struct TransientAnalysisResult {
    std::vector<TransientPoint> trajectory;
};

struct DcSweepPoint {
    std::vector<double> source_values;
    std::vector<double> x;
    int newton_iterations;
};

struct DcSweepAnalysisResult {
    std::vector<std::string> source_names;
    std::vector<DcSweepPoint> points;
};

using SimulationResult =
    std::variant<OperatingPointResult,
                 TransientAnalysisResult,
                 DcSweepAnalysisResult>;

SimulationResult simulate(const Circuit& circuit);

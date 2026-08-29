#include "simulate.h"

#include "mna/MnaSystem.h"
#include "parser/Circuit.h"
#include "solver/newton.h"
#include "solver/transient.h"

#include <stdexcept>
#include <utility>
#include <variant>
#include <vector>

SimulationResult simulate(const Circuit& circuit) {
    std::vector<Device*> device_view;
    device_view.reserve(circuit.devices.size());
    for (const auto& ptr : circuit.devices) {
        device_view.push_back(ptr.get());
    }
    MnaSystem sys{circuit.nodes, circuit.num_branch_unknowns};
    if (circuit.analysis_type == AnalysisType::Op) {
        NewtonResult newton_result = newton_solve(device_view, sys);
        return OperatingPointResult{std::move(newton_result.x), newton_result.iterations};
    } else if (circuit.analysis_type == AnalysisType::Tran) {
        std::vector<double> initial_x(sys.dim(), 0.0);
        return TransientAnalysisResult{transient_solve(device_view, sys, circuit.t_step, circuit.t_stop, initial_x)};
    } else {
        throw std::invalid_argument("Analysis type is illegal!");
    }
}
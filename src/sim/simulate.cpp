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
    if (circuit.device_names.size() != circuit.devices.size()) {
        throw std::invalid_argument(
            "Circuit device names must correspond one-to-one with devices");
    }
    std::vector<Device*> device_view;
    device_view.reserve(circuit.devices.size());
    for (const auto& ptr : circuit.devices) {
        device_view.push_back(ptr.get());
    }
    MnaSystem sys{circuit.nodes, circuit.num_branch_unknowns};
    if (std::holds_alternative<OperatingPointAnalysis>(circuit.analysis)) {
        NewtonResult newton_result = newton_solve(device_view, sys);
        return OperatingPointResult{std::move(newton_result.x), newton_result.iterations};
    }
    if (const auto* transient =
            std::get_if<TransientAnalysis>(&circuit.analysis)) {
        std::vector<double> initial_x(sys.dim(), 0.0);
        return TransientAnalysisResult{transient_solve(
            device_view,
            sys,
            transient->t_step,
            transient->t_stop,
            initial_x)};
    }
    if (std::holds_alternative<DcSweepAnalysis>(circuit.analysis)) {
        throw std::invalid_argument("DC sweep controller is not implemented");
    }
    throw std::invalid_argument("Analysis directive is missing");
}

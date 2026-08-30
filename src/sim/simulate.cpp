#include "simulate.h"

#include "mna/MnaSystem.h"
#include "devices/IndependentSource.h"
#include "parser/Circuit.h"
#include "solver/newton.h"
#include "solver/transient.h"

#include <iomanip>
#include <limits>
#include <locale>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace {
const IndependentSource& resolve_dc_source(
    const Circuit& circuit,
    const DcSweepAxis& axis) {
    if (axis.device_index >= circuit.devices.size()
        || axis.device_index >= circuit.device_names.size()) {
        throw std::invalid_argument(
            "DC sweep source index is outside Circuit devices");
    }
    if (circuit.device_names[axis.device_index] != axis.source_name) {
        throw std::invalid_argument(
            "DC sweep source name does not match its device index");
    }
    const auto* source = dynamic_cast<const IndependentSource*>(
        circuit.devices[axis.device_index].get());
    if (source == nullptr) {
        throw std::invalid_argument(
            "DC sweep target is not an independent source");
    }
    return *source;
}

[[noreturn]] void throw_dc_point_failure(
    std::size_t point_index,
    const std::vector<std::string>& source_names,
    const std::vector<double>& source_values,
    const std::exception& cause) {
    std::ostringstream message;
    message.imbue(std::locale::classic());
    message << std::setprecision(std::numeric_limits<double>::max_digits10)
            << "DC sweep point " << point_index + 1 << " failed at ";
    for (std::size_t index = 0; index < source_names.size(); ++index) {
        if (index != 0) {
            message << ", ";
        }
        message << source_names[index] << '=' << source_values[index];
    }
    message << ": " << cause.what();
    throw std::runtime_error(message.str());
}

DcSweepAnalysisResult simulate_dc_sweep(
    const Circuit& circuit,
    const DcSweepAnalysis& analysis,
    const std::vector<Device*>& base_device_view,
    MnaSystem& sys) {
    const IndependentSource& primary_source =
        resolve_dc_source(circuit, analysis.primary);
    const IndependentSource* secondary_source = nullptr;
    if (analysis.secondary) {
        if (analysis.primary.device_index
            == analysis.secondary->device_index) {
            throw std::invalid_argument(
                "DC sweep sources must refer to different devices");
        }
        secondary_source = &resolve_dc_source(circuit, *analysis.secondary);
    }

    const std::vector<double> primary_values = generate_dc_sweep_values(
        analysis.primary.start,
        analysis.primary.stop,
        analysis.primary.step);
    const std::vector<double> secondary_values = analysis.secondary
        ? generate_dc_sweep_values(
            analysis.secondary->start,
            analysis.secondary->stop,
            analysis.secondary->step)
        : std::vector<double>{0.0};

    if (!secondary_values.empty()
        && primary_values.size()
            > std::numeric_limits<std::size_t>::max()
                / secondary_values.size()) {
        throw std::overflow_error("DC sweep point count overflowed");
    }

    DcSweepAnalysisResult result;
    result.source_names.push_back(analysis.primary.source_name);
    if (analysis.secondary) {
        result.source_names.push_back(analysis.secondary->source_name);
    }
    result.points.reserve(primary_values.size() * secondary_values.size());

    std::vector<double> initial_x(
        static_cast<std::size_t>(sys.dim()),
        0.0);
    for (double secondary_value : secondary_values) {
        for (double primary_value : primary_values) {
            std::vector<Device*> point_device_view = base_device_view;
            std::unique_ptr<IndependentSource> primary_override =
                primary_source.clone_with_dc_value(primary_value);
            point_device_view[analysis.primary.device_index] =
                primary_override.get();

            std::unique_ptr<IndependentSource> secondary_override;
            std::vector<double> source_values{primary_value};
            if (analysis.secondary) {
                secondary_override =
                    secondary_source->clone_with_dc_value(secondary_value);
                point_device_view[analysis.secondary->device_index] =
                    secondary_override.get();
                source_values.push_back(secondary_value);
            }

            NewtonResult point_result;
            try {
                point_result = newton_solve(
                    point_device_view,
                    sys,
                    initial_x);
            } catch (const std::exception& cause) {
                throw_dc_point_failure(
                    result.points.size(),
                    result.source_names,
                    source_values,
                    cause);
            }
            initial_x = point_result.x;
            result.points.push_back(DcSweepPoint{
                std::move(source_values),
                std::move(point_result.x),
                point_result.iterations,
            });
        }
    }
    return result;
}
} // namespace

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
    if (const auto* dc = std::get_if<DcSweepAnalysis>(&circuit.analysis)) {
        return simulate_dc_sweep(circuit, *dc, device_view, sys);
    }
    throw std::invalid_argument("Analysis directive is missing");
}

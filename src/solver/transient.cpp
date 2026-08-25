#include "devices/Device.h"
#include "newton.h"

#include "transient.h"
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <cmath>

std::vector<TransientPoint> transient_solve(const std::vector<Device*>& devices, MnaSystem& sys, double t_step, double t_stop, const std::vector<double>& initial_x, double newton_tol, int newton_max_iter) {
    if (t_step <= 0) {
        std::stringstream e;
        e << "t_step must be positive";
        throw std::invalid_argument(e.str());
    }
    if (t_stop < 0) {
        std::stringstream e;
        e << "t_stop can not be negative!";
        throw std::invalid_argument(e.str());
    }

    if (initial_x.size() != static_cast<size_t>(sys.dim())) {
        std::stringstream e;
        e << "initial_x.size() must be equal to sys.dim()!";
        throw std::invalid_argument(e.str());
    }
    double q = t_stop / t_step;
    int rounded_steps = std::round(q);
    if (std::abs(q - rounded_steps) > 1e-12) {
        std::stringstream e;
        e << "the t_stop must be an integer multiple of t_step!";
        throw std::invalid_argument(e.str());
    }
    size_t num_steps = static_cast<size_t>(rounded_steps);

    std::vector<TransientPoint> trajectory = {{0.0, initial_x}};
    std::vector<double> x_prev = initial_x;
    for (size_t step = 1; step <= num_steps; step++) {
        double cur_time = step * t_step;
        TransientContext tmp_ctx{t_step, x_prev, cur_time};
        NewtonResult tmp_res = newton_solve(devices, sys, x_prev, tmp_ctx, newton_tol, newton_max_iter);
        trajectory.push_back({cur_time, tmp_res.x});
        x_prev = tmp_res.x;
    }
    return trajectory;
}
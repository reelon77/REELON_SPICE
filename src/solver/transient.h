#pragma once

#include "devices/Device.h"
#include "mna/MnaSystem.h"
#include <vector>

struct TransientPoint {
    double time;
    std::vector<double> x;
};

std::vector<TransientPoint> transient_solve(
    const std::vector<Device*>& devices,
    MnaSystem& sys,
    double t_step,
    double t_stop,
    const std::vector<double>& initial_x,
    double newton_tol = 1e-9,
    int newton_max_iter = 300);
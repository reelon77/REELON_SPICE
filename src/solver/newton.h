#pragma once

#include "devices/Device.h"
#include "mna/MnaSystem.h"
#include <vector>


struct NewtonResult {
    std::vector<double> x;
    int iterations;
};

NewtonResult newton_solve(
    const std::vector<Device*>& devices,
    MnaSystem& sys,
    double tol = 1e-9,
    int max_iter = 300);

NewtonResult newton_solve(
    const std::vector<Device*>& devices,
    MnaSystem& sys,
    const std::vector<double>& initial_x,
    double tol = 1e-9,
    int max_iter = 300);

NewtonResult newton_solve(
    const std::vector<Device*>& devices,
    MnaSystem& sys,
    const std::vector<double>& initial_x,
    const TransientContext& context,
    double tol = 1e-9,
    int max_iter = 300);

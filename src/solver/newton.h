#pragma once

#include "devices/Device.h"
#include "mna/MnaSystem.h"
#include <vector>


struct NewtonResult {
    std::vector<double> x;
    int iterations;
};

NewtonResult newton_solve(const std::vector<Device*>& devices, MnaSystem& sys, double tol = 1e-9, int max_iter = 300);
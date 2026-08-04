#pragma once

#include "devices/Device.h"
#include "mna/MnaSystem.h"
#include "core/LU.h"
#include <vector>
#include <cmath>
#include <numeric>

struct NewtonResult {
    std::vector<double> res;
    int iterations;
};

NewtonResult newton_solve(const std::vector<Device*>& devices, MnaSystem& sys, double tol, int max_iter);
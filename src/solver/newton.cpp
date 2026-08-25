#include "newton.h"
#include "core/LU.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {
    NewtonResult newton_solve_impl(const std::vector<Device*>& devices, MnaSystem& sys, const std::vector<double>& initial_x, const TransientContext* context, double tol, int max_iter) {
        if (initial_x.size() != static_cast<size_t>(sys.dim())) {
            std::stringstream e;
            e << "initial_x.size() is not equal to sys.dim()!";
            throw std::invalid_argument(e.str());
        }
        std::vector<double> x = initial_x;

        for (int iter = 0; iter < max_iter; ++iter) {
            sys.clear();
            for (const auto& device : devices) {
                if (context) {
                    device->stamp(sys, x, *context);
                } else {
                    device->stamp(sys, x);
                }
            }
            auto lu = lu_decomposition(sys.get_A());
            auto x_new = lu_solve(lu, sys.get_b());
            // 无穷范数
            double diff = std::transform_reduce(
        x_new.begin(), x_new.end(), x.begin(), 0.0,
        [](double m, double d) { return std::max(m, d); },   // 归约:打擂台
        [](double a, double b) { return std::abs(a - b); }); // 变换:逐分量 |差|
            if (diff < tol) {
                return {x_new, iter + 1}; // 收敛
            }
            x = x_new;
        }
        throw std::runtime_error("Newton-Raphson method did not converge"); // 不收敛
    }
}

NewtonResult newton_solve(    const std::vector<Device*>& devices, MnaSystem& sys, double tol, int max_iter) {
    std::vector<double> x(sys.dim(), 0.0);
    return newton_solve(devices, sys, x, tol, max_iter);
}

NewtonResult newton_solve(const std::vector<Device*>& devices, MnaSystem& sys, const std::vector<double>& initial_x, double tol, int max_iter) {
    return newton_solve_impl(devices, sys, initial_x, nullptr, tol, max_iter);
}

NewtonResult newton_solve(const std::vector<Device*>& devices, MnaSystem& sys, const std::vector<double>& initial_x, const TransientContext& context, double tol, int max_iter) {
    return newton_solve_impl(devices, sys, initial_x, &context, tol, max_iter);
}

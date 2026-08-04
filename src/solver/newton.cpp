#include "newton.h"
#include <algorithm>

NewtonResult newton_solve(const std::vector<Device*>& devices, MnaSystem& sys, double tol, int max_iter) {
    std::vector<double> x(sys.dim(), 0.0); // 初始解向量
    for (int iter = 0; iter < max_iter; ++iter) {
        sys.clear();
        for (const auto& device : devices) {
            device->stamp(sys, x);
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
#pragma once

#include <vector>
#include "Matrix.h"
#include <stdexcept>
#include <optional>

struct LUResult {
    Matrix L, U;
    std::vector<int> perm;   // L·U = P·A，perm[i] 表示 P·A 的第 i 行取自 A 的第 perm[i] 行
};

std::optional<int>  find_nonzero_row(const Matrix& A, int col, int start_row);
void exchange_rows(Matrix& A, int row1, int row2);

LUResult lu_decomposition_naked(const Matrix& A, Matrix& L, Matrix& U);
LUResult lu_decomposition(const Matrix& A, Matrix& L, Matrix& U);

// 用已有的分解结果解 A·x = b（A 是分解时传入的那个矩阵）
std::vector<double> lu_solve(const LUResult& lu, const std::vector<double>& b);
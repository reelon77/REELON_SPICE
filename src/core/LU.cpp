#include "LU.h"
#include <algorithm>
#include <optional>

// 查找第一个可交换的行
std::optional<int> find_nonzero_row(const Matrix& A, int col, int start_row) {
    for (int i = start_row; i < A.rows(); ++i) {
        if (A(i, col) != 0) {
            return i;
        }
    }
    return std::nullopt; // 没有找到非零行
}

// 换行操作
void exchange_rows(Matrix& A, int row1, int row2) {
    if (row1 < 0 || row1 >= A.rows() || row2 < 0 || row2 >= A.rows()) {
        throw std::out_of_range("Row index out of range.");
    }

    for (int j = 0; j < A.cols(); ++j) {
        std::swap(A(row1, j), A(row2, j));
    }
}


// 裸LU分解（不查主元，不查奇异）
LUResult lu_decomposition_naked(const Matrix& A, Matrix& L, Matrix& U) {
    // 检查输入矩阵是否为方阵
    if (A.rows() != A.cols()) {
        throw std::invalid_argument("Input matrix must be square.");
    }

    Matrix A_copy = A; // 创建A的副本，以便在不修改原始矩阵的情况下进行操作

    // 对每行遍历
    for (int it_row = 0; it_row < A_copy.rows(); ++it_row) {
        if (A_copy(it_row, it_row) == 0) {
            std::optional<int> nonzero_row = find_nonzero_row(A_copy, it_row, it_row + 1);
            if (nonzero_row) {
                exchange_rows(A_copy, it_row, *nonzero_row);
            } else {
                throw std::runtime_error("Matrix is singular; cannot perform LU decomposition.");
            }
        }
        // 对每列遍历
        for (int it_col = 0; it_col < A_copy.cols(); ++it_col) {
            if (it_row > it_col) {
                L(it_row, it_col) = A_copy(it_row, it_col) / A_copy(it_col, it_col);
                for (int k = 0; k < A_copy.cols(); ++k) {
                    A_copy(it_row, k) -= L(it_row, it_col) * A_copy(it_col, k);
                }
            } else if (it_row == it_col) {
                L(it_row, it_col) = 1.0; // 对角线元素为1
                U(it_row, it_col) = A_copy(it_row, it_col);
            } else {
                U(it_row, it_col) = A_copy(it_row, it_col);
            }
        }
    }
    LUResult result{L, U, {}};
    result.perm.resize(A.rows());
    for (int i = 0; i < A.rows(); ++i) {
        result.perm[i] = i; // 这里假设没有行交换，实际应用中应根据行交换情况更新 perm
    }
    return result;
}

LUResult lu_decomposition(const Matrix& A, Matrix& L, Matrix& U) {
    
}
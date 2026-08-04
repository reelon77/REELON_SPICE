#include "LU.h"
#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>

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

// 带部分选主元（partial pivoting）的 LU 分解，Doolittle 形式（L 对角为 1）
//
// 与 naked 版的两个关键区别：
//   1. 主元不是"第一个非零"，而是当前列剩余行里 |值| 最大的那个 —— 乘数
//      m = A(i,k)/A(k,k) 因此恒有 |m| ≤ 1，消元不会放大舍入误差（数值稳定性）
//   2. 循环是列优先（k 在外层）：每轮先定第 k 列主元、再拿它消下面所有行，
//      所以"主元定稿"和"检查主元"发生在同一时刻，不会出现 naked 版那种
//      "检查时非零、消元后才变零"的漏网
LUResult lu_decomposition(const Matrix& A) {
    if (A.rows() != A.cols()) {
        throw std::invalid_argument("Input matrix must be square.");
    }
    Matrix L(A.rows(), A.cols());
    Matrix U(A.rows(), A.cols());
    const int n = A.rows();

    // 主元阈值：电路里"节点悬空 / 整个网络未接地"会让 G 矩阵奇异。必须在这里
    // 拦下来报错，而不是让 1/0 变成 inf 一路传到节点电压里
    constexpr double kPivotEps = 1e-12;

    // 就地分解的工作副本：消元过程中，L 的乘数直接写回下三角空出来的位置
    Matrix work = A;

    // perm[i] = P·A 的第 i 行取自 A 的第几行；初始为恒等置换
    std::vector<int> perm(n);
    for (int i = 0; i < n; ++i) {
        perm[i] = i;
    }

    for (int k = 0; k < n; ++k) {
        // 1) 在第 k 列的 k..n-1 行中挑 |值| 最大的行作主元
        int pivot_row = k;
        for (int i = k + 1; i < n; ++i) {
            if (std::abs(work(i, k)) > std::abs(work(pivot_row, k))) {
                pivot_row = i;
            }
        }

        // 2) 最大的都接近 0 ⇒ 该列在剩余行里全为 0 ⇒ 矩阵奇异
        if (std::abs(work(pivot_row, k)) < kPivotEps) {
            throw std::runtime_error("Matrix is singular; cannot perform LU decomposition.");
        }

        // 3) 换行。整行交换是关键：前几轮算好的 L 乘数就存在这一行的左半边，
        //    必须跟着一起换。（L/U 分开存两个矩阵时，这一步极易漏掉）
        if (pivot_row != k) {
            exchange_rows(work, k, pivot_row);
            std::swap(perm[k], perm[pivot_row]);
        }

        // 4) 消元：乘数存回 work(i,k)（L 的位置），只更新右侧未处理的列
        for (int i = k + 1; i < n; ++i) {
            const double m = work(i, k) / work(k, k);
            work(i, k) = m;
            for (int j = k + 1; j < n; ++j) {
                work(i, j) -= m * work(k, j);
            }
        }
    }

    // 5) 把就地结果拆成独立的 L（单位下三角）与 U（上三角）。
    //    显式写 0，不依赖 Matrix 构造函数把元素初始化成零
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i > j) {
                L(i, j) = work(i, j);
                U(i, j) = 0.0;
            } else if (i == j) {
                L(i, j) = 1.0;
                U(i, j) = work(i, j);
            } else {
                L(i, j) = 0.0;
                U(i, j) = work(i, j);
            }
        }
    }

    return LUResult{L, U, perm};
}

// 用分解结果解 A·x = b：重排 → 前代 → 回代
std::vector<double> lu_solve(const LUResult& lu, const std::vector<double>& b) {
    const int n = lu.L.rows();
    if (static_cast<int>(b.size()) != n || static_cast<int>(lu.perm.size()) != n) {
        throw std::invalid_argument("lu_solve: b/perm size must match the factorization.");
    }

    // 1) 重排出 P·b。分解时 exchange_rows 换过行，L/U 对应的是 P·A 的行序，
    //    b 必须换到同一行序上，否则解的是 A·x = P⁻¹b —— 方程根本不对。
    //    必须写进独立向量：原地 b[i] = b[perm[i]] 会边读边写、自我破坏。
    std::vector<double> y(n);
    for (int i = 0; i < n; ++i) {
        y[i] = b[lu.perm[i]];
    }

    // 2) 前代解 L·y = P·b。L 的对角线是隐含的 1，所以这里不除 L(i,i)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            y[i] -= lu.L(i, j) * y[j];
        }
    }

    // 3) 回代解 U·x = y。U 的对角线是真实主元，必须除。
    //    重排只在第 1 步做一次，回代完直接返回，不能再排第二次
    for (int i = n - 1; i >= 0; --i) {
        for (int j = i + 1; j < n; ++j) {
            y[i] -= lu.U(i, j) * y[j];
        }
        y[i] /= lu.U(i, i);
    }
    return y;
}
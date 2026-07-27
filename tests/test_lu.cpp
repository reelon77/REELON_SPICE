#include "core/LU.h"
#include <gtest/gtest.h>
#include <cmath>
#include <initializer_list>
#include <vector>

// LU 模块夹具：提供构造矩阵和验证分解性质的公共工具
class LUTest : public ::testing::Test {
protected:
    static constexpr double kTol = 1e-12;

    static Matrix make(int n, std::initializer_list<double> v) {
        Matrix M(n, n);
        int idx = 0;
        for (double x : v) {
            M(idx / n, idx % n) = x;
            ++idx;
        }
        return M;
    }

    static Matrix multiply(const Matrix& L, const Matrix& U) {
        int n = L.rows();
        Matrix P(n, n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                double s = 0;
                for (int k = 0; k < n; ++k) {
                    s += L(i, k) * U(k, j);
                }
                P(i, j) = s;
            }
        }
        return P;
    }

    static void expect_unit_lower_triangular(const Matrix& L) {
        for (int i = 0; i < L.rows(); ++i) {
            EXPECT_NEAR(L(i, i), 1.0, kTol) << "L(" << i << "," << i << ") 应为 1";
            for (int j = i + 1; j < L.cols(); ++j) {
                EXPECT_NEAR(L(i, j), 0.0, kTol) << "L 上三角应为 0";
            }
        }
    }

    static void expect_upper_triangular(const Matrix& U) {
        for (int i = 0; i < U.rows(); ++i) {
            for (int j = 0; j < i; ++j) {
                EXPECT_NEAR(U(i, j), 0.0, kTol) << "U 下三角应为 0";
            }
        }
    }

    static void expect_all_finite(const Matrix& M) {
        for (int i = 0; i < M.rows(); ++i) {
            for (int j = 0; j < M.cols(); ++j) {
                EXPECT_TRUE(std::isfinite(M(i, j)))
                    << "M(" << i << "," << j << ") = " << M(i, j);
            }
        }
    }

    // L·U 的行集合应恰好是 A 的行集合的一个重排（分解带换行时 L·U = P·A）
    static void expect_rows_are_permutation_of(const Matrix& LU, const Matrix& A) {
        int n = A.rows();
        std::vector<bool> used(n, false);
        for (int i = 0; i < n; ++i) {
            bool matched = false;
            for (int r = 0; r < n && !matched; ++r) {
                if (used[r]) continue;
                bool same = true;
                for (int j = 0; j < n; ++j) {
                    if (std::abs(LU(i, j) - A(r, j)) > kTol) {
                        same = false;
                        break;
                    }
                }
                if (same) {
                    used[r] = true;
                    matched = true;
                }
            }
            EXPECT_TRUE(matched) << "L*U 的第 " << i << " 行在 A 中找不到对应行";
        }
    }
};

// 良态矩阵：不需要换行，应满足 A = L·U，L 单位下三角，U 上三角
TEST_F(LUTest, WellConditionedReconstructsA) {
    Matrix A = make(3, {2, 1, 1,
                        4, 3, 3,
                        8, 7, 9});
    Matrix L(3, 3), U(3, 3);
    lu_decomposition_naked(A, L, U);

    expect_unit_lower_triangular(L);
    expect_upper_triangular(U);
    Matrix P = multiply(L, U);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            EXPECT_NEAR(P(i, j), A(i, j), kTol) << "(" << i << "," << j << ")";
        }
    }
}

// 非奇异，但 A(1,1) 消完第 0 列后变 0：必须换行处理，结果不允许出现 inf/nan
TEST_F(LUTest, PivotBecomesZeroMidway) {
    Matrix A = make(3, {1, 1, 0,
                        1, 1, 1,
                        0, 1, 1});
    Matrix L(3, 3), U(3, 3);
    lu_decomposition_naked(A, L, U);

    expect_all_finite(L);
    expect_all_finite(U);
    expect_rows_are_permutation_of(multiply(L, U), A);
}

// 开头就需要换行：A(0,0) = 0
TEST_F(LUTest, NeedsRowSwapAtStart) {
    Matrix A = make(3, {0, 1, 2,
                        1, 2, 3,
                        3, 1, 1});
    Matrix L(3, 3), U(3, 3);
    lu_decomposition_naked(A, L, U);

    expect_all_finite(L);
    expect_all_finite(U);
    expect_rows_are_permutation_of(multiply(L, U), A);
}

// 奇异矩阵应抛异常，而不是静默返回
TEST_F(LUTest, SingularMatrixThrows) {
    Matrix A = make(2, {1, 2,
                        2, 4});
    Matrix L(2, 2), U(2, 2);
    EXPECT_THROW(lu_decomposition_naked(A, L, U), std::runtime_error);
}

// 非方阵应抛异常
TEST_F(LUTest, NonSquareThrows) {
    Matrix A(2, 3);
    Matrix L(2, 2), U(2, 2);
    EXPECT_THROW(lu_decomposition_naked(A, L, U), std::invalid_argument);
}

// --- 辅助函数 ---

TEST_F(LUTest, FindNonzeroRowFindsFirstMatch) {
    Matrix A = make(3, {0, 1, 0,
                        0, 2, 0,
                        3, 0, 0});
    EXPECT_EQ(find_nonzero_row(A, 0, 0), std::optional<int>(2));
    EXPECT_EQ(find_nonzero_row(A, 1, 0), std::optional<int>(0));
    EXPECT_EQ(find_nonzero_row(A, 1, 1), std::optional<int>(1)); // start_row 生效
}

TEST_F(LUTest, FindNonzeroRowReturnsNulloptWhenAllZero) {
    Matrix A = make(2, {0, 1,
                        0, 1});
    EXPECT_EQ(find_nonzero_row(A, 0, 0), std::nullopt);
}

TEST_F(LUTest, ExchangeRowsSwapsWholeRows) {
    Matrix A = make(2, {1, 2,
                        3, 4});
    exchange_rows(A, 0, 1);
    EXPECT_EQ(A(0, 0), 3.0);
    EXPECT_EQ(A(0, 1), 4.0);
    EXPECT_EQ(A(1, 0), 1.0);
    EXPECT_EQ(A(1, 1), 2.0);
}

TEST_F(LUTest, ExchangeRowsRejectsBadIndex) {
    Matrix A(2, 2);
    EXPECT_THROW(exchange_rows(A, 0, 5), std::out_of_range);
}

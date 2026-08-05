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

// 【对照用例：记录 naked 版已知缺陷，不修】
// A 非奇异，但 A(1,1) 在消完第 0 列后才变 0。naked 版的主元检查发生在
// 本行消元之前——检查时 A(1,1)=1 非零，于是漏检；下一行拿 0 当除数，
// L(2,1)=inf、U(2,2)=-inf 静默写入，不抛异常。
// 正式版按"消元后的当前值"选主元，同一矩阵的正确行为见
// PivotLUTest.PivotBecomesZeroMidway。
TEST_F(LUTest, PivotBecomesZeroMidwayKnownDefect) {
    Matrix A = make(3, {1, 1, 0,
                        1, 1, 1,
                        0, 1, 1});
    Matrix L(3, 3), U(3, 3);
    EXPECT_NO_THROW(lu_decomposition_naked(A, L, U));

    // 缺陷证据。若下面两条开始失败，说明 naked 版被修复，本用例应同步退役
    EXPECT_TRUE(std::isinf(L(2, 1))) << "漏检主元应产生 inf 乘数";
    EXPECT_TRUE(std::isinf(U(2, 2))) << "inf 应扩散进 U";
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

// 【对照用例：记录 naked 版已知缺陷，不修】
// A 奇异（第 1 行 = 2×第 0 行），本应抛异常。但检查在消元前：检查时
// A(1,1)=4 非零，消元后才变 0，于是静默通过，U(1,1)=0 直接流向下游
// ——lu_solve 回代时就是除零。正确行为见 PivotLUTest.SingularMatrixThrows。
TEST_F(LUTest, SingularMatrixPassesSilentlyKnownDefect) {
    Matrix A = make(2, {1, 2,
                        2, 4});
    Matrix L(2, 2), U(2, 2);
    EXPECT_NO_THROW(lu_decomposition_naked(A, L, U));

    // 缺陷证据：奇异性以"U 对角线上的 0"的形式静默留存
    EXPECT_EQ(U(1, 1), 0.0);
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

// ============================================================
// lu_decomposition：带部分选主元的正式版
// ============================================================

class PivotLUTest : public LUTest {
protected:
    // 比 expect_rows_are_permutation_of 更严：perm 必须精确对得上，
    // 即 (L·U) 的第 i 行 == A 的第 perm[i] 行
    static void expect_LU_equals_PA(const LUResult& r, const Matrix& A) {
        Matrix LU = multiply(r.L, r.U);
        int n = A.rows();
        ASSERT_EQ(static_cast<int>(r.perm.size()), n);
        for (int i = 0; i < n; ++i) {
            ASSERT_GE(r.perm[i], 0);
            ASSERT_LT(r.perm[i], n);
            for (int j = 0; j < n; ++j) {
                EXPECT_NEAR(LU(i, j), A(r.perm[i], j), kTol)
                    << "(L*U)(" << i << "," << j << ") 应等于 A(" << r.perm[i] << "," << j << ")";
            }
        }
    }

    // 部分选主元的标志性性质：所有乘数 |L(i,j)| ≤ 1
    static void expect_multipliers_bounded(const Matrix& L) {
        for (int i = 0; i < L.rows(); ++i) {
            for (int j = 0; j < i; ++j) {
                EXPECT_LE(std::abs(L(i, j)), 1.0 + kTol)
                    << "L(" << i << "," << j << ") = " << L(i, j) << " 超过 1，选主元没生效";
            }
        }
    }
};

TEST_F(PivotLUTest, WellConditionedReconstructsPA) {
    Matrix A = make(3, {2, 1, 1,
                        4, 3, 3,
                        8, 7, 9});
    LUResult r = lu_decomposition(A);

    expect_unit_lower_triangular(r.L);
    expect_upper_triangular(r.U);
    expect_LU_equals_PA(r, A);
    expect_multipliers_bounded(r.L);
}

// naked 版在这里会产生 inf/nan：主元消元后才变 0
TEST_F(PivotLUTest, PivotBecomesZeroMidway) {
    Matrix A = make(3, {1, 1, 0,
                        1, 1, 1,
                        0, 1, 1});
    LUResult r = lu_decomposition(A);

    expect_all_finite(r.L);
    expect_all_finite(r.U);
    expect_LU_equals_PA(r, A);
}

TEST_F(PivotLUTest, NeedsRowSwapAtStart) {
    Matrix A = make(3, {0, 1, 2,
                        1, 2, 3,
                        3, 1, 1});
    LUResult r = lu_decomposition(A);

    expect_all_finite(r.L);
    expect_all_finite(r.U);
    expect_LU_equals_PA(r, A);
    EXPECT_EQ(r.perm[0], 2) << "首列 |3| 最大，应选第 2 行，而不是第一个非零的第 1 行";
}

// 选的是绝对值最大，不是第一个非零 —— 这正是与 naked 版的分水岭
TEST_F(PivotLUTest, PicksLargestMagnitudeNotFirstNonzero) {
    Matrix A = make(2, {1, 2,
                        100, 3});
    LUResult r = lu_decomposition(A);

    EXPECT_EQ(r.perm[0], 1);
    EXPECT_NEAR(r.U(0, 0), 100.0, kTol);
    EXPECT_NEAR(r.L(1, 0), 0.01, kTol); // 1/100，而非 100/1
    expect_multipliers_bounded(r.L);
    expect_LU_equals_PA(r, A);
}

TEST_F(PivotLUTest, SingularMatrixThrows) {
    Matrix A = make(2, {1, 2,
                        2, 4});
    EXPECT_THROW(lu_decomposition(A), std::runtime_error);
}

// 未接地电路：G 是带权图拉普拉斯，行和为 0 ⇒ 奇异 ⇒ 必须报错而不是给 inf
TEST_F(PivotLUTest, UngroundedLaplacianThrows) {
    Matrix G = make(2, {0.001, -0.001,
                        -0.001, 0.001});
    EXPECT_THROW(lu_decomposition(G), std::runtime_error);
}

// 对照：节点 2 经 R2 接地后，行和不再为 0，G 非奇异，正常分解
TEST_F(PivotLUTest, GroundedGMatrixIsNonsingular) {
    Matrix G = make(2, {0.001, -0.001,
                        -0.001, 0.0015});
    LUResult r = lu_decomposition(G);

    expect_all_finite(r.L);
    expect_all_finite(r.U);
    expect_LU_equals_PA(r, G);
}

TEST_F(PivotLUTest, NonSquareThrows) {
    Matrix A(2, 3);
    EXPECT_THROW(lu_decomposition(A), std::invalid_argument);
}

// ============================================================
// lu_solve：前代 + 回代
// ============================================================

class SolveTest : public LUTest {
protected:
    static std::vector<double> mul(const Matrix& A, const std::vector<double>& x) {
        std::vector<double> r(A.rows(), 0.0);
        for (int i = 0; i < A.rows(); ++i) {
            for (int j = 0; j < A.cols(); ++j) {
                r[i] += A(i, j) * x[j];
            }
        }
        return r;
    }

    // 每个用例查两条**互相独立**的判据：
    //   1. 与手推答案比对   —— 验"答案对不对"
    //   2. 残差 |A·x − b|   —— 验"方程解没解对"
    // 两者的失败指向完全不同的地方：只有 1 红而 2 绿，说明是手推答案抄错了，
    // 不是求解器坏了。（本项目已经两次栽在错误的手推答案上，故必须双判据。）
    static void expect_solves(const Matrix& A,
                              const std::vector<double>& b,
                              const std::vector<double>& expect) {
        const int n = A.rows();
        LUResult r = lu_decomposition(A);
        std::vector<double> x = lu_solve(r, b);

        ASSERT_EQ(x.size(), expect.size());
        for (int i = 0; i < n; ++i) {
            EXPECT_NEAR(x[i], expect[i], kTol) << "x[" << i << "] 与手推答案不符";
        }

        std::vector<double> Ax = mul(A, x);
        for (int i = 0; i < n; ++i) {
            EXPECT_NEAR(Ax[i], b[i], kTol) << "残差：(A*x)[" << i << "] != b[" << i << "]";
        }
    }

    // 置换里存在长度 > 2 的循环。单次对调的逆等于它自身，
    // 因此只有长循环才能验出 y[i]=b[perm[i]] 有没有被写成反方向。
    static bool has_long_cycle(const std::vector<int>& perm) {
        int n = static_cast<int>(perm.size());
        std::vector<bool> seen(n, false);
        for (int s = 0; s < n; ++s) {
            if (seen[s]) continue;
            int len = 0, cur = s;
            while (!seen[cur]) {
                seen[cur] = true;
                cur = perm[cur];
                ++len;
            }
            if (len > 2) return true;
        }
        return false;
    }
};

// 交接单的 2×2 手推用例。perm 为恒等，不涉及重排
TEST_F(SolveTest, TwoByTwoNoPivoting) {
    expect_solves(make(2, {2, 1,
                           1, 3}), {5, 10}, {1, 3});
}

// 关键用例：perm 是 3-循环 [2,0,1]。若把 y[i]=b[perm[i]] 写成
// y[perm[i]]=b[i]（逆置换），此用例会挂，而单次对调的用例察觉不到
TEST_F(SolveTest, ThreeByThreeWithThreeCyclePermutation) {
    Matrix A = make(3, {1, 3, 2,
                        2, 1, 5,
                        4, 1, 0});
    LUResult r = lu_decomposition(A);
    ASSERT_TRUE(has_long_cycle(r.perm))
        << "该用例必须产生长循环置换才有意义，否则测不出正/逆写反";

    expect_solves(A, {13, 19, 6}, {1, 2, 3});
}

TEST_F(SolveTest, FourByFourMultipleSwaps) {
    expect_solves(make(4, {1, 2, 0, 1,
                           0, 1, 3, 2,
                           5, 0, 1, 1,
                           2, 3, 1, 0}),
                  {-2.5, 8, 8.5, -1}, {1, -2, 3, 0.5});
}

TEST_F(SolveTest, NegativeCoefficientsAndFractionalSolution) {
    expect_solves(make(3, {0, -4, 2,
                           3, 1, -1,
                           1, 2, 6}),
                  {-6, 5.5, 11.5}, {1.5, 2, 1});
}

TEST_F(SolveTest, SingleElement) {
    expect_solves(make(1, {5}), {10}, {2});
}

// 回归测试：曾经 perm 完全没被使用，解出来的是 A·x = P⁻¹b。
// 这里 b[0] != b[1]，重排不是空操作 —— 若漏掉重排，A*x 会等于 b 的前两项对调
TEST_F(SolveTest, PermutationIsActuallyApplied) {
    Matrix A = make(3, {1, 2, 1,
                        3, 1, 1,
                        2, 1, 3});
    expect_solves(A, {5, 6, 9}, {1, 1, 2});
}

// 反面对照：同一个 A，但 b[0] == b[1]，重排恰好退化成恒等。
// 这种 b 永远测不出"漏掉重排"的 bug —— 保留它是为了记住这个测试设计陷阱
TEST_F(SolveTest, PermutationDegeneratesWhenSwappedEntriesAreEqual) {
    Matrix A = make(3, {1, 2, 1,
                        3, 1, 1,
                        2, 1, 3});
    expect_solves(A, {8, 8, 13}, {1, 2, 3});
}

TEST_F(SolveTest, MismatchedRhsSizeThrows) {
    Matrix A = make(2, {2, 1,
                        1, 3});
    LUResult r = lu_decomposition(A);
    EXPECT_THROW(lu_solve(r, std::vector<double>{1, 2, 3}), std::invalid_argument);
}

#include "core/Matrix.h"
#include <gtest/gtest.h>

// Matrix 模块夹具：准备一个 2x3 矩阵，元素按行填 1..6
class MatrixTest : public ::testing::Test {
protected:
    void SetUp() override {
        for (int i = 0; i < m_.rows(); ++i) {
            for (int j = 0; j < m_.cols(); ++j) {
                m_(i, j) = i * m_.cols() + j + 1;
            }
        }
    }

    Matrix m_{2, 3};
};

TEST_F(MatrixTest, ReportsDimensions) {
    EXPECT_EQ(m_.rows(), 2);
    EXPECT_EQ(m_.cols(), 3);
}

TEST_F(MatrixTest, ZeroInitializedOnConstruction) {
    Matrix z(2, 2);
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            EXPECT_EQ(z(i, j), 0.0);
        }
    }
}

TEST_F(MatrixTest, ReadWriteRoundtrip) {
    EXPECT_EQ(m_(0, 0), 1.0);
    EXPECT_EQ(m_(1, 2), 6.0);
    m_(1, 2) = 42.0;
    EXPECT_EQ(m_(1, 2), 42.0);
}

TEST_F(MatrixTest, ConstAccessReadsSameValues) {
    const Matrix& cm = m_;
    EXPECT_EQ(cm(0, 1), 2.0);
    EXPECT_EQ(cm(1, 0), 4.0);
}

TEST_F(MatrixTest, CopyIsDeep) {
    Matrix copy = m_;
    copy(0, 0) = 99.0;
    EXPECT_EQ(m_(0, 0), 1.0); // 改副本不影响原矩阵
}

#pragma once

#include <vector>

class Matrix {
public:
    Matrix() = delete;
    Matrix(int i, int j);
    // 运算符重载，读写
    double& operator()(int i, int j);
    double operator()(int i, int j) const;

    int rows() const;
    int cols() const;

private:
    int rows_ = 0;
    int cols_ = 0;
    std::vector<double> matrix_;
};
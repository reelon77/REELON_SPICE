#pragma once

#include <vector>

class Matrix {
public:
    Matrix() = delete;
    Matrix(int i, int j);
    explicit Matrix(int i);
    // 运算符重载，读写
    double& operator()(int i, int j);
    double operator()(int i, int j) const;

    int rows() const;
    int cols() const;

private:
    int rows_;
    int cols_;
    std::vector<double> matrix_;
};
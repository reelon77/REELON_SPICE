#pragma once

#include <vector>

class Matrix {
public:
    Matrix() = delete;
    Matrix(int i, int j);
    // 运算符重载，读写
    double& operator()(int i, int j);
    std::vector<double> getRow(int i) const;
    std::vector<double> getCol(int i) const;

private:
    int rows = 0;
    int cols = 0;
    std::vector<double> matrix;
};
#include "Matrix.h"

Matrix::Matrix(int i, int j) : rows_(i), cols_(j), matrix_(i * j) {}

double& Matrix::operator()(int i, int j) {
    return matrix_[i * cols_ + j];
}

double Matrix::operator()(int i, int j) const {
    return matrix_[i * cols_ + j];
}

int Matrix::rows() const {
    return rows_;
}

int Matrix::cols() const {
    return cols_;
}
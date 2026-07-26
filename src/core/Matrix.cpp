#include "Matrix.h"

Matrix::Matrix(int i, int j) : rows(i), cols(j), matrix(i * j) {}

double& Matrix::operator()(int i, int j) {
    return matrix[i * cols + j];
}

std::vector<double> Matrix::getRow(int i) const {
    std::vector<double> row(cols);
    for (int j = 0; j < cols; ++j) {
        row[j] = (this->matrix)[i * cols + j];
    }
    return row;
}

std::vector<double> Matrix::getCol(int i) const {
    std::vector<double> col(rows);
    for (int j = 0; j < rows; ++j) {
        col[j] = (this->matrix)[j * cols + i];
    }
    return col;
}
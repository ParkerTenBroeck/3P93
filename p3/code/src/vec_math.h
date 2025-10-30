#ifndef VEC_MATH_H
#define VEC_MATH_H
#include <array>
#include <cmath>
#include <iostream>

#include "util/types.h"

template<typename T, usize R, usize C>
class Matrix;

template<typename T>
using Matrix4 = Matrix<T, 4, 4>;
template<typename T>
using Matrix3 = Matrix<T, 3, 3>;
template<typename T>
using Matrix2 = Matrix<T, 2, 2>;
template<typename T>
using Matrix1 = Matrix<T, 1, 1>;


template<typename T, size_t N>
using Vector = Matrix<T, N, 1>;

template<typename T>
using Vector4 = Vector<T, 4>;
template<typename T>
using Vector3 = Vector<T, 3>;
template<typename T>
using Vector2 = Vector<T, 2>;
template<typename T>
using Vector1 = Vector<T, 1>;


template<typename T, usize R, usize C>
class Matrix {
public:
    std::array<std::array<T, R>, C> data;
    INLINE Matrix() {
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                data[c][r] = 0;
            }
        }
    }

    INLINE Matrix(std::initializer_list<T> row) {
        usize j  = 0;
        for (auto i = row.begin(); j < R * C && i != row.end(); j++, ++i) {
            data[j%C][j/C] = *i;
        }
    }

    INLINE explicit Matrix(ref<std::array<std::array<T, C>, R>> data) : data(data) {}

    template<usize RR, usize CC>
    INLINE explicit Matrix(Matrix<T, RR, CC> data) : Matrix() {
        for (usize r = 0; r < std::min(R, RR); r++) {
            for (usize c = 0; c < std::min(C, CC); c++) {
                (*this)[{r, c}] = data[{r, c}];
            }
        }
    }

    [[nodiscard]]
    INLINE ref_mut<T> operator[](const Vector2<usize> row_column) {
        return data[row_column.y()][row_column.x()];
    }

    [[nodiscard]]
    INLINE ref<T> operator[](const Vector2<usize> row_column) const {
        return data[row_column.y()][row_column.x()];
    }

    [[nodiscard]]
    INLINE ref_mut<T> operator[](usize r) {
        static_assert(C == 1, "must be a vector to use this");
        return (*this).data[0][r];
    }

    [[nodiscard]]
    INLINE ref<T> operator[](usize r) const {
        static_assert(C == 1, "must be a vector to use this");
        return (*this).data[0][r];
    }

    template<usize P>
    [[nodiscard]]
    INLINE friend auto operator*(ref<Matrix> m1, ref<Matrix<T, C, P>> m2) {
        Matrix<T, R, P> result{};
        for (usize r = 0; r < R; r++) {
            for (usize p = 0; p < P; p++) {
                for (usize c = 0; c < C; c++) {
                    result[{r, p}] += m1[{r, c}] * m2[{c, p}];
                }
            }
        }

        return result;
    }

    [[nodiscard]]
    INLINE friend auto operator-(ref<Matrix> lhs, ref<Matrix> rhs) {
        Matrix result;
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result[{r, c}] = lhs[{r, c}] - rhs[{r, c}];
            }
        }
        return result;
    }

    [[nodiscard]]
    INLINE friend auto operator-(ref<Matrix> rhs) {
        Matrix result;
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result[{r, c}] = -rhs[{r, c}];
            }
        }
        return result;
    }

    [[nodiscard]]
    INLINE friend auto operator+(ref<Matrix> lhs, ref<Matrix> rhs) {
        Matrix result;
        for (usize c = 0; c < C; c++) {
            for (usize r = 0; r < R; r++) {
                result[{r, c}] = lhs[{r, c}] + rhs[{r, c}];
            }
        }
        return result;
    }


    [[nodiscard]]
    INLINE friend Matrix operator/(ref<Matrix> lhs, T scaler) {
        Matrix result;
        for (usize c = 0; c < C; c++) {
            for (usize r = 0; r < R; r++) {
                result[{r, c}] = lhs[{r, c}] / scaler;
            }
        }
        return result;
    }

    [[nodiscard]]
    INLINE Matrix add_scalar(T x) const {
        Matrix result;
        for (usize c = 0; c < C; c++) {
            for (usize r = 0; r < R; r++) {
                result[{r, c}] = (*this)[{r, c}] + x;
            }
        }
        return result;
    }

    [[nodiscard]]
    INLINE friend auto operator*(ref<Matrix> lhs, T scaler) -> Matrix {
        Matrix result;
        for (usize c = 0; c < C; c++) {
            for (usize r = 0; r < R; r++) {
                result[{r, c}] = lhs[{r, c}] * scaler;
            }
        }
        return result;
    };

    [[nodiscard]]
    INLINE friend Matrix operator*(T scaler, ref<Matrix> rhs) {
        return rhs * scaler;
    };

    void print() const {
        for (size_t r = 0; r < R; r++) {
            for (size_t c = 0; c < C; c++) {
                std::cout << (*this)[{r, c}] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    [[nodiscard]]
    INLINE Matrix mult_components(ref<Matrix> rhs) const {
        Matrix result;
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result[{r, c}] = (*this)[{r, c}] * rhs[{r, c}];
            }
        }
        return result;
    }

    [[nodiscard]]
    Matrix inverse() const {
        static_assert(R==C, "Matrix must have the same size");
        Matrix A = *this;
        Matrix I;

        for (usize i = 0; i < R; ++i) {
            for (usize j = 0; j < C; ++j) {
                I.data[j][i] = (i == j) ? static_cast<T>(1) : static_cast<T>(0);
            }
        }

        for (usize i = 0; i < R; ++i) {
            T pivot = A.data[i][i];
            if (std::abs(pivot) < static_cast<T>(1e-12)) {
                bool swapped = false;
                for (usize k = i + 1; k < R; ++k) {
                    if (std::abs(A.data[i][k]) > static_cast<T>(1e-12)) {
                        for (usize c = 0; c < C; ++c) {
                            std::swap(A.data[c][i], A.data[c][k]);
                            std::swap(I.data[c][i], I.data[c][k]);
                        }
                        pivot = A.data[i][i];
                        swapped = true;
                        break;
                    }
                }
                if (!swapped) {
                    throw std::runtime_error("Matrix is singular and cannot be inverted.");
                }
            }

            for (usize c = 0; c < C; ++c) {
                A.data[c][i] /= pivot;
                I.data[c][i] /= pivot;
            }

            for (usize r = 0; r < R; ++r) {
                if (r == i) continue;
                T factor = A.data[i][r];
                for (usize c = 0; c < C; ++c) {
                    A.data[c][r] -= factor * A.data[c][i];
                    I.data[c][r] -= factor * I.data[c][i];
                }
            }
        }
        return I;
    }

    [[nodiscard]]
    INLINE Matrix<T, R+1, C> extend(T value) const {
        static_assert(C == 1, "must be a vector to use this");
        Matrix<T, R+1, C> extended(*this);
        extended[{R, 0}] = value;
        return extended;
    }

    [[nodiscard]]
    INLINE Matrix<T, C, R> transpose() const {
        Matrix<T, C, R> result;
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result[{c, r}] = (*this)[{r, c}];
            }
        }
        return result;
    }

    [[nodiscard]]
    static Matrix4<T> projection(T near, T far, T aspect, T fov) {
        return {
            1/(aspect*std::tan(fov/2)), 0, 0, 0,
            0, 1/std::tan(fov/2), 0, 0,
            0, 0, -(far+near)/(far-near), -2*far*near/(far-near),
            0, 0, -1, 0
        };
    }

    [[nodiscard]]
    static Matrix4<T> look_at(Vector3<T> pos, Vector3<T> target, Vector3<T> up) {

        auto forward = (pos-target).normalize();
        auto right = up.cross(forward).normalize();
        up = forward.cross(right).normalize();
        return Matrix4<f32>({
            right.x(), up.x(), forward.x(), 0,
            right.y(), up.y(), forward.y(), 0,
            right.z(), up.z(), forward.z(), 0,
            -right.dot(pos), -up.dot(pos), -forward.dot(pos), 1
        }).transpose();
    }

    [[nodiscard]]
    INLINE ref<T> x() const {
        static_assert(R >= 1 && C == 1, "must be a vector with length larger than 0");
        return (*this)[0];
    }

    [[nodiscard]]
    INLINE ref_mut<T> x() {
        static_assert(R >= 1 && C == 1, "must be a vector with length larger than 0");
        return (*this)[0];
    }

    [[nodiscard]]
    INLINE ref<T> y() const {
        static_assert(R >= 2 && C == 1, "must be a vector with length larger than 1");
        return (*this)[1];
    }

    [[nodiscard]]
    INLINE ref_mut<T> y() {
        static_assert(R >= 2 && C == 1, "must be a vector with length larger than 1");
        return (*this)[1];
    }

    [[nodiscard]]
    INLINE ref<T> z() const {
        static_assert(R >= 3 && C == 1, "must be a vector with length larger than 2");
        return (*this)[2];
    }

    [[nodiscard]]
    INLINE ref_mut<T> z() {
        static_assert(R >= 3 && C == 1, "must be a vector with length larger than 2");
        return (*this)[2];
    }

    [[nodiscard]]
    INLINE ref<T> w() const {
        static_assert(R >= 4 && C == 1, "must be a vector with length larger than 3");
        return (*this)[3];
    }

    [[nodiscard]]
    INLINE ref_mut<T> w() {
        static_assert(R >= 4 && C == 1, "must be a vector with length larger than 3");
        return (*this)[3];
    }

    [[nodiscard]]
    INLINE Vector2<T> xy() const {
        static_assert(R >= 2 && C == 1, "must be a vector with length larger than 1");
        return {this->x(), this->y()};
    }

    [[nodiscard]]
    INLINE Vector3<T> xyz() const {
        static_assert(R >= 3 && C == 1, "must be a vector with length larger than 2");
        return {this->x(), this->y(), this->z()};
    }

    [[nodiscard]]
    INLINE Matrix normalize() const {
        Matrix result{};
        T mag = 1/this->magnitude();
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result[{r, c}] = (*this)[{r, c}] * mag;
            }
        }
        return result;
    }

    [[nodiscard]]
    INLINE T magnitude_squared() const {
        T result = 0;
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result += (*this)[{r, c}] * (*this)[{r, c}];
            }
        }
        return result;
    }

    [[nodiscard]]
    INLINE T magnitude() const {
        return std::sqrt(this->magnitude_squared());
    }

    [[nodiscard]]
    INLINE T dot(ref<Matrix> other) const {
        T result = 0.;
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result += (*this)[{r, c}] * other[{r, c}];
            }
        }
        return result;
    }

    [[nodiscard]]
    INLINE Vector3<T> cross(ref<Vector3<T>> other) const {
        return {
            this->y() * other.z() - this->z() * other.y(),
            this->z() * other.x() - this->x() * other.z(),
            this->x() * other.y() - this->y() * other.x(),
        };
    }

    [[nodiscard]]
    static Matrix2<T> rotation(T theta) {
        return Matrix2<T>{
            std::cos(theta), -std::sin(theta),
            std::sin(theta), std::cos(theta)
        };
    }

    [[nodiscard]]
    static Matrix3<T> rotation(Vector3<T> ypr) {
        return Matrix::rotation(ypr.x(), ypr.y(), ypr.z());
    }

    [[nodiscard]]
    static Matrix3<T> rotation(T yaw, T pitch, T roll) {
        return Matrix3<T>{
            std::cos(yaw), -std::sin(yaw), 0,
            std::sin(yaw), std::cos(yaw), 0,
            0, 0, 1
        }*Matrix3<T>{
            std::cos(pitch), 0, std::sin(pitch),
            0, 1, 0,
            -std::sin(pitch), 0, std::cos(pitch),
        }*Matrix3<T>{
            1, 0, 0,
            0, std::cos(roll), -std::sin(roll),
            0, std::sin(roll), std::cos(roll)
        };
    }
};

#endif //VEC_MATH_H

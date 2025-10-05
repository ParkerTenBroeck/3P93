//
// Created by may on 03/10/25.
//

#ifndef VEC_MATH_H
#define VEC_MATH_H
#include <array>
#include <cmath>
#include <iostream>

#include "types.h"

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
    Matrix() {
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                data[c][r] = 0;
            }
        }
    }

    Matrix(std::initializer_list<T> row) {
        for (auto i = row.begin(), j = 0ull; j < R * C && i != row.end(); j++, ++i) {
            data[j/R][j%R] = *i;
        }
    }

    explicit Matrix(ref<std::array<std::array<T, C>, R>> data) : data(data) {}

    template<usize RR, usize CC>
    explicit Matrix(Matrix<T, RR, CC> data) : Matrix() {
        for (int r = 0; r < std::min(R, RR); r++) {
            for (int c = 0; c < std::min(C, CC); c++) {
                this[{r, c}] = data[{r, c}];
            }
        }
    }

    [[nodiscard]]
    ref_mut<T> operator[](std::array<usize, 2> row_column) {
        return data[row_column[1]][row_column[0]];
    }

    [[nodiscard]]
    ref<T> operator[](std::array<usize, 2> row_column) const {
        return data[row_column[1]][row_column[0]];
    }

    [[nodiscard]]
    ref_mut<T> operator[](usize r) {
        static_assert(C == 1, "must be a vector to use this");
        return (*this).data[0][r];
    }

    [[nodiscard]]
    ref<T> operator[](usize r) const {
        static_assert(C == 1, "must be a vector to use this");
        return (*this).data[0][r];
    }

    template<usize P>
    friend auto operator*(ref<Matrix> m1, ref<Matrix<T, C, P>> m2) {
        Matrix<T, R, P> result{};
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                for (usize p = 0; p < P; p++) {
                    result[{r, p}] += m1[{r, c}] * m2[{c, p}];
                }
            }
        }

        return result;
    }

    friend auto operator-(ref<Matrix> lhs, ref<Matrix> rhs) {
        Matrix result{};
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result[{r, c}] += lhs[{r, c}] - rhs[{r, c}];
            }
        }
        return result;
    }

    friend auto operator+(ref<Matrix> lhs, ref<Matrix> rhs) {
        Matrix result{};
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result[{r, c}] += lhs[{r, c}] + rhs[{r, c}];
            }
        }
        return result;
    }

    void print() const {
        for (size_t r = 0; r < R; r++) {
            for (size_t c = 0; c < C; c++) {
                std::cout << (*this)[{r, c}] << " ";
            }
            std::cout << std::endl;
        }
    }

    static Matrix4<T> perspective(T near, T far, T aspect, T fov) {
        return {
            1/aspect*tan(fov/2), 0, 0, 0,
            0, 1/tan(fov/2), 0, 0,
            0, 0, -(far+near)/(far-near), -2*far*near/(far-near),
            0, 0, -1, 0
        };
    }

    static Matrix4<T> look_at(Vector3<T> pos, Vector3<T> target, Vector3<T> up) {
        auto forward = (target-pos).normalize();
        auto right = forward.cross(up).normalize();
        auto upN = up.normalize();
        Matrix4<T> l{
            right.x(), right.y(), right.z(), 0,
            upN.x(), upN.y(), upN.z(), 0,
            forward.x(), forward.y(), forward.z(), 0,
            0, 0, 0, 1
        };
        Matrix4<T> t{
            1, 0, 0, -pos.x(),
            0, 1, 0, -pos.y(),
            0, 0, 1, -pos.z(),
            0, 0, 0, 1
        };
        return l*t;
    }

    T x() const {
        static_assert(R >= 1 && C == 1, "must be a vector with length larger than 0");
        return (*this)[0];
    }

    T y() const {
        static_assert(R >= 2 && C == 1, "must be a vector with length larger than 1");
        return (*this)[1];
    }

    T z() const {
        static_assert(R >= 3 && C == 1, "must be a vector with length larger than 2");
        return (*this)[2];
    }

    T w() const {
        static_assert(R >= 4 && C == 1, "must be a vector with length larger than 3");
        return (*this)[3];
    }

    Vector2<T> xy() const {
        static_assert(R >= 2 && C == 1, "must be a vector with length larger than 1");
        return {this.x(), this.y()};
    }

    Vector3<T> xyz() const {
        static_assert(R >= 3 && C == 1, "must be a vector with length larger than 2");
        return {this.x(), this.y(), this.z()};
    }

    Matrix normalize() const {
        Matrix result{};
        T mag = this->magnitude();
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result[{r, c}] += (*this)[{r, c}] / mag;
            }
        }
        return result;
    }

    T magnitude_squared() const {
        T result = 0;
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result += (*this)[{r, c}] * (*this)[{r, c}];
            }
        }
        return result;
    }

    T magnitude() const {
        return std::sqrt(this->magnitude_squared());
    }

    T dot(ref<Matrix> other) const {
        T result = 0.;
        for (usize r = 0; r < R; r++) {
            for (usize c = 0; c < C; c++) {
                result += (*this)[{r, c}] * other[{r, c}];
            }
        }
        return result;
    }

    Vector3<T> cross(ref<Vector3<T>> other) const {
        return {
            (*this).y() * other.z() - (*this).z() * other.y(),
            (*this).z() * other.x() - (*this).x() * other.z(),
            (*this).x() * other.y() - (*this).y() * other.x(),
        };
    }

    static Matrix2<T> rotation(T theta) {
        return Matrix2{
            cos(theta), -sin(theta),
            sin(theta), cos(theta)
        };
    }

    static Matrix3<T> rotation(Vector3<T> ypr) {
        return Matrix::rotation(ypr.x(), ypr.y(), ypr.z());
    }

    static Matrix3<T> rotation(T yaw, T pitch, T roll) {
        return Matrix3{
            cos(yaw), -sin(yaw), 0,
            sin(yaw), cos(yaw), 0,
            0, 0, 1
        }*Matrix3{
            cos(pitch), 0, sin(pitch),
            0, 1, 0,
            -sin(pitch), 0, cos(pitch),
        }*Matrix3{
            1, 0, 0,
            0, cos(roll), -sin(roll),
            0, sin(roll), cos(roll)
        };
    }
};







#endif //VEC_MATH_H

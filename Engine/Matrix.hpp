#pragma once

#include <array>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <limits>
#include <Vector.hpp>

namespace Uma_Math
{
	template <typename T>
	class Matrix2x2
	{
	public:
		// Core constructors and Rule of 5
		constexpr Matrix2x2(T m00 = T{}, T m01 = T{}, T m10 = T{}, T m11 = T{}) : pData{m00, m01, m10, m11} {}
		constexpr Matrix2x2(const Matrix2x2& other) = default;
		constexpr Matrix2x2(Matrix2x2&& other) noexcept = default;
		constexpr Matrix2x2& operator=(const Matrix2x2& other) = default;
		constexpr Matrix2x2& operator=(Matrix2x2&& other) noexcept = default;
		~Matrix2x2() = default;

		// Template conversion constructor
		template<typename U>
		constexpr explicit Matrix2x2(const Matrix2x2<U>& other) : pData{static_cast<T>(other.pData[0]), static_cast<T>(other.pData[1]), static_cast<T>(other.pData[2]), static_cast<T>(other.pData[3])} {}

		// Element access (row, column)
		constexpr T& operator()(std::size_t row, std::size_t col) { return pData[row * 2 + col]; }
		constexpr const T& operator()(std::size_t row, std::size_t col) const { return pData[row * 2 + col]; }

		// Linear array-style access
		constexpr T& operator[](std::size_t index) { return pData[index]; }
		constexpr const T& operator[](std::size_t index) const { return pData[index]; }

        // Compound assignment operators
        constexpr Matrix2x2& operator+=(const Matrix2x2& other)
        {
            for (std::size_t i = 0; i < 4; ++i) {
                pData[i] += other.pData[i];
            }
            return *this;
        }

        constexpr Matrix2x2& operator-=(const Matrix2x2& other)
        {
            for (std::size_t i = 0; i < 4; ++i) {
                pData[i] -= other.pData[i];
            }
            return *this;
        }

        template<typename U>
        constexpr Matrix2x2& operator*=(U scalar)
        {
            for (auto& element : pData) {
                element *= scalar;
            }
            return *this;
        }

        template<typename U>
        constexpr Matrix2x2& operator/=(U scalar)
        {
            for (auto& element : pData) {
                element /= scalar;
            }
            return *this;
        }

        constexpr Matrix2x2& operator*=(const Matrix2x2& other)
        {
            *this = *this * other;  // Uses non-member operator*
            return *this;
        }

        // Matrix operations
        constexpr T determinant() const
        {
            return pData[0] * pData[3] - pData[1] * pData[2];
        }

        constexpr Matrix2x2 transpose() const
        {
            return Matrix2x2(pData[0], pData[2], pData[1], pData[3]);
        }

        // Static factory methods
        static constexpr Matrix2x2 identity()
        {
            return Matrix2x2(T{ 1 }, T{}, T{}, T{ 1 });
        }

        static constexpr Matrix2x2 zero()
        {
            return Matrix2x2();
        }

	private:
		union
		{
			std::array<T, 4> pData;
			struct
			{
				T m00, m01;
				T m10, m11;
			};
		};

        // Friend declarations for non-member functions that need private access
        template<typename U> friend constexpr bool operator==(const Matrix2x2<U>& lhs, const Matrix2x2<U>& rhs);
        template<typename U> friend std::ostream& operator<<(std::ostream& os, const Matrix2x2<U>& mat);
	};

    template<typename T>
    class Matrix3x3
    {
    public:
        // Core constructors and Rule of 5
        constexpr Matrix3x3(T m00 = T{}, T m01 = T{}, T m02 = T{}, T m10 = T{}, T m11 = T{}, T m12 = T{}, T m20 = T{}, T m21 = T{}, T m22 = T{}) : pData{m00, m01, m02, m10, m11, m12, m20, m21, m22} {}
        constexpr Matrix3x3(const Matrix3x3& other) = default;
        constexpr Matrix3x3(Matrix3x3&& other) noexcept = default;
        constexpr Matrix3x3& operator=(const Matrix3x3& other) = default;
        constexpr Matrix3x3& operator=(Matrix3x3&& other) noexcept = default;
        ~Matrix3x3() = default;

        // Template conversion constructor
        template<typename U>
        constexpr explicit Matrix3x3(const Matrix3x3<U>& other)
            : pData{static_cast<T>(other.pData[0]), static_cast<T>(other.pData[1]), static_cast<T>(other.pData[2]),
                   static_cast<T>(other.pData[3]), static_cast<T>(other.pData[4]), static_cast<T>(other.pData[5]),
                   static_cast<T>(other.pData[6]), static_cast<T>(other.pData[7]), static_cast<T>(other.pData[8])} {}

        // Element access (row, column)
        constexpr T& operator()(std::size_t row, std::size_t col) { return pData[row * 3 + col]; }
        constexpr const T& operator()(std::size_t row, std::size_t col) const { return pData[row * 3 + col]; }

        // Linear array-style access
        constexpr T& operator[](std::size_t index) { return pData[index]; }
        constexpr const T& operator[](std::size_t index) const { return pData[index]; }

        // Compound assignment operators
        constexpr Matrix3x3& operator+=(const Matrix3x3& other)
        {
            for (std::size_t i = 0; i < 9; ++i) {
                pData[i] += other.pData[i];
            }
            return *this;
        }

        constexpr Matrix3x3& operator-=(const Matrix3x3& other)
        {
            for (std::size_t i = 0; i < 9; ++i) {
                pData[i] -= other.pData[i];
            }
            return *this;
        }

        template<typename U>
        constexpr Matrix3x3& operator*=(U scalar)
        {
            for (auto& element : pData) {
                element *= scalar;
            }
            return *this;
        }

        template<typename U>
        constexpr Matrix3x3& operator/=(U scalar)
        {
            for (auto& element : pData) {
                element /= scalar;
            }
            return *this;
        }

        constexpr Matrix3x3& operator*=(const Matrix3x3& other)
        {
            *this = *this * other;  // Uses non-member operator*
            return *this;
        }

        // Matrix operations
        constexpr T determinant() const
        {
            return pData[0] * (pData[4] * pData[8] - pData[5] * pData[7]) -
                   pData[1] * (pData[3] * pData[8] - pData[5] * pData[6]) +
                   pData[2] * (pData[3] * pData[7] - pData[4] * pData[6]);
        }

        constexpr Matrix3x3 transpose() const
        {
            return Matrix3x3(pData[0], pData[3], pData[6],
                             pData[1], pData[4], pData[7],
                             pData[2], pData[5], pData[8]);
        }

        // Static factory methods
        static constexpr Matrix3x3 identity()
        {
            return Matrix3x3(T{ 1 }, T{}, T{}, T{}, T{ 1 }, T{}, T{}, T{}, T{ 1 });
        }

        static constexpr Matrix3x3 zero()
        {
            return Matrix3x3();
        }

    private:
        union
        {
            std::array<T, 9> pData;
            struct
            {
                T m00, m01, m02;
                T m10, m11, m12;
                T m20, m21, m22;
            };
        };

        // Friend declarations for non-member functions that need private access
        template<typename U> friend constexpr bool operator==(const Matrix3x3<U>& lhs, const Matrix3x3<U>& rhs);
        template<typename U> friend std::ostream& operator<<(std::ostream& os, const Matrix3x3<U>& mat);
    };

    // Arithmetic operators for Matrix2x2
    template<typename T>
    constexpr Matrix2x2<T> operator+(const Matrix2x2<T>& lhs, const Matrix2x2<T>& rhs)
    {
        return Matrix2x2<T>(lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2], lhs[3] + rhs[3]);
    }

    template<typename T>
    constexpr Matrix2x2<T> operator-(const Matrix2x2<T>& lhs, const Matrix2x2<T>& rhs)
    {
        return Matrix2x2<T>(lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2], lhs[3] - rhs[3]);
    }

    template<typename T, typename U>
    constexpr Matrix2x2<T> operator*(const Matrix2x2<T>& mat, U scalar)
    {
        return Matrix2x2<T>(mat[0] * scalar, mat[1] * scalar, mat[2] * scalar, mat[3] * scalar);
    }

    template<typename T, typename U>
    constexpr Matrix2x2<T> operator*(U scalar, const Matrix2x2<T>& mat)
    {
        return mat * scalar;
    }

    template<typename T, typename U>
    constexpr Matrix2x2<T> operator/(const Matrix2x2<T>& mat, U scalar)
    {
        return Matrix2x2<T>(mat[0] / scalar, mat[1] / scalar, mat[2] / scalar, mat[3] / scalar);
    }

    // Matrix multiplication for Matrix2x2
    template<typename T>
    constexpr Matrix2x2<T> operator*(const Matrix2x2<T>& lhs, const Matrix2x2<T>& rhs)
    {
        return Matrix2x2<T>(
            lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0),
            lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1),
            lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0),
            lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1)
        );
    }

    // Arithmetic operators for Matrix3x3
    template<typename T>
    constexpr Matrix3x3<T> operator+(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
    {
        Matrix3x3<T> result;
        for (std::size_t i = 0; i < 9; ++i) {
            result[i] = lhs[i] + rhs[i];
        }
        return result;
    }

    template<typename T>
    constexpr Matrix3x3<T> operator-(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
    {
        Matrix3x3<T> result;
        for (std::size_t i = 0; i < 9; ++i) {
            result[i] = lhs[i] - rhs[i];
        }
        return result;
    }

    template<typename T, typename U>
    constexpr Matrix3x3<T> operator*(const Matrix3x3<T>& mat, U scalar)
    {
        Matrix3x3<T> result;
        for (std::size_t i = 0; i < 9; ++i) {
            result[i] = mat[i] * scalar;
        }
        return result;
    }

    template<typename T, typename U>
    constexpr Matrix3x3<T> operator*(U scalar, const Matrix3x3<T>& mat)
    {
        return mat * scalar;
    }

    template<typename T, typename U>
    constexpr Matrix3x3<T> operator/(const Matrix3x3<T>& mat, U scalar)
    {
        Matrix3x3<T> result;
        for (std::size_t i = 0; i < 9; ++i) {
            result[i] = mat[i] / scalar;
        }
        return result;
    }

    // Matrix multiplication for Matrix3x3
    template<typename T>
    constexpr Matrix3x3<T> operator*(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
    {
        return Matrix3x3<T>(
            lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0) + lhs(0, 2) * rhs(2, 0),
            lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1) + lhs(0, 2) * rhs(2, 1),
            lhs(0, 0) * rhs(0, 2) + lhs(0, 1) * rhs(1, 2) + lhs(0, 2) * rhs(2, 2),
            lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0) + lhs(1, 2) * rhs(2, 0),
            lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1) + lhs(1, 2) * rhs(2, 1),
            lhs(1, 0) * rhs(0, 2) + lhs(1, 1) * rhs(1, 2) + lhs(1, 2) * rhs(2, 2),
            lhs(2, 0) * rhs(0, 0) + lhs(2, 1) * rhs(1, 0) + lhs(2, 2) * rhs(2, 0),
            lhs(2, 0) * rhs(0, 1) + lhs(2, 1) * rhs(1, 1) + lhs(2, 2) * rhs(2, 1),
            lhs(2, 0) * rhs(0, 2) + lhs(2, 1) * rhs(1, 2) + lhs(2, 2) * rhs(2, 2)
        );
    }

    // Unary minus operators
    template<typename T>
    constexpr Matrix2x2<T> operator-(const Matrix2x2<T>& mat)
    {
        return Matrix2x2<T>(-mat[0], -mat[1], -mat[2], -mat[3]);
    }

    template<typename T>
    constexpr Matrix3x3<T> operator-(const Matrix3x3<T>& mat)
    {
        Matrix3x3<T> result;
        for (std::size_t i = 0; i < 9; ++i) {
            result[i] = -mat[i];
        }
        return result;
    }

    // Comparison operators
    template<typename T>
    constexpr bool operator==(const Matrix2x2<T>& lhs, const Matrix2x2<T>& rhs)
    {
        for (std::size_t i = 0; i < 4; ++i) {
            if (lhs.pData[i] != rhs.pData[i]) return false;
        }
        return true;
    }

    template<typename T>
    constexpr bool operator!=(const Matrix2x2<T>& lhs, const Matrix2x2<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template<typename T>
    constexpr bool operator==(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
    {
        for (std::size_t i = 0; i < 9; ++i) {
            if (lhs.pData[i] != rhs.pData[i]) return false;
        }
        return true;
    }

    template<typename T>
    constexpr bool operator!=(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
    {
        return !(lhs == rhs);
    }

    // Matrix-Vector multiplication
    template<typename T>
    constexpr Vector2D<T> operator*(const Matrix2x2<T>& mat, const Vector2D<T>& vec)
    {
        return Vector2D<T>(
            mat(0, 0) * vec.x() + mat(0, 1) * vec.y(),
            mat(1, 0) * vec.x() + mat(1, 1) * vec.y()
        );
    }

    template<typename T>
    constexpr Vector3D<T> operator*(const Matrix3x3<T>& mat, const Vector3D<T>& vec)
    {
        return Vector3D<T>(
            mat(0, 0) * vec.x() + mat(0, 1) * vec.y() + mat(0, 2) * vec.z(),
            mat(1, 0) * vec.x() + mat(1, 1) * vec.y() + mat(1, 2) * vec.z(),
            mat(2, 0) * vec.x() + mat(2, 1) * vec.y() + mat(2, 2) * vec.z()
        );
    }

    // Inverse operations (for floating point types)
    template<typename T>
    auto inverse(const Matrix2x2<T>& mat) -> std::enable_if_t<std::is_floating_point_v<T>, Matrix2x2<T>>
    {
        T det = mat.determinant();
        if (std::abs(det) < std::numeric_limits<T>::epsilon()) {
            throw std::runtime_error("Matrix is not invertible, determinant is zero");
        }

        T inv_det = T{ 1 } / det;
        return Matrix2x2<T>(
            mat(1, 1) * inv_det, -mat(0, 1) * inv_det,
            -mat(1, 0) * inv_det, mat(0, 0) * inv_det
        );
    }

    template<typename T>
    auto inverse(const Matrix3x3<T>& mat) -> std::enable_if_t<std::is_floating_point_v<T>, Matrix3x3<T>>
    {
        T det = mat.determinant();
        if (std::abs(det) < std::numeric_limits<T>::epsilon()) {
            throw std::runtime_error("Matrix is not invertible, determinant is zero");
        }

        T inv_det = T{ 1 } / det;
        return Matrix3x3<T>(
            (mat(1, 1) * mat(2, 2) - mat(1, 2) * mat(2, 1)) * inv_det,
            (mat(0, 2) * mat(2, 1) - mat(0, 1) * mat(2, 2)) * inv_det,
            (mat(0, 1) * mat(1, 2) - mat(0, 2) * mat(1, 1)) * inv_det,
            (mat(1, 2) * mat(2, 0) - mat(1, 0) * mat(2, 2)) * inv_det,
            (mat(0, 0) * mat(2, 2) - mat(0, 2) * mat(2, 0)) * inv_det,
            (mat(0, 2) * mat(1, 0) - mat(0, 0) * mat(1, 2)) * inv_det,
            (mat(1, 0) * mat(2, 1) - mat(1, 1) * mat(2, 0)) * inv_det,
            (mat(0, 1) * mat(2, 0) - mat(0, 0) * mat(2, 1)) * inv_det,
            (mat(0, 0) * mat(1, 1) - mat(0, 1) * mat(1, 0)) * inv_det
        );
    }

    // Stream operators
    template<typename T>
    std::ostream& operator<<(std::ostream& os, const Matrix2x2<T>& mat)
    {
        os << "[" << mat.pData[0] << ", " << mat.pData[1] << "]\n";
        os << "[" << mat.pData[2] << ", " << mat.pData[3] << "]";
        return os;
    }

    template<typename T>
    std::ostream& operator<<(std::ostream& os, const Matrix3x3<T>& mat)
    {
        os << "[" << mat.pData[0] << ", " << mat.pData[1] << ", " << mat.pData[2] << "]\n";
        os << "[" << mat.pData[3] << ", " << mat.pData[4] << ", " << mat.pData[5] << "]\n";
        os << "[" << mat.pData[6] << ", " << mat.pData[7] << ", " << mat.pData[8] << "]";
        return os;
    }
}
#pragma once

#include <array>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>
#include <limits>
#include "Vector.h"

namespace Uma_Math
{
    // Column proxy for matrix[col][row] access
    template <typename T, std::size_t Rows>
    class ColumnProxy
    {
    public:
        constexpr ColumnProxy(T* data) : m_data(data) {}

        constexpr T& operator[](std::size_t row) noexcept
        {
            return m_data[row];
        }

        constexpr const T& operator[](std::size_t row) const noexcept
        {
            return m_data[row];
        }

    private:
        T* m_data;
    };

    template <typename T, std::size_t Rows>
    class ConstColumnProxy
    {
    public:
        constexpr ConstColumnProxy(const T* data) : m_data(data) {}

        constexpr const T& operator[](std::size_t row) const noexcept
        {
            return m_data[row];
        }

    private:
        const T* m_data;
    };

    template <typename T = float>
    class Matrix2x2
    {
    public:
        // Core constructors and Rule of 5
        constexpr Matrix2x2(T m00 = T{}, T m01 = T{}, T m10 = T{}, T m11 = T{}) : m_data{m00, m10, m01, m11} {}
        constexpr Matrix2x2(const Vector2D<T>& col0, const Vector2D<T>& col1) : m_data{col0.x, col0.y, col1.x, col1.y} {}
        constexpr Matrix2x2(const Matrix2x2& other) = default;
        constexpr Matrix2x2(Matrix2x2&& other) noexcept = default;
        constexpr Matrix2x2& operator=(const Matrix2x2& other) = default;
        constexpr Matrix2x2& operator=(Matrix2x2&& other) noexcept = default;
        ~Matrix2x2() = default;

        // Template conversion constructor
        template <typename U>
        constexpr explicit Matrix2x2(const Matrix2x2<U>& other)
            : m_data{static_cast<T>(other.get(0)), static_cast<T>(other.get(1)),
                     static_cast<T>(other.get(2)), static_cast<T>(other.get(3))} {
        }

        // GLM-style column access: matrix[col][row]
        constexpr ColumnProxy<T, 2> operator[](std::size_t col) noexcept { return ColumnProxy<T, 2>(&m_data[col * 2]); }
        constexpr ConstColumnProxy<T, 2> operator[](std::size_t col) const noexcept { return ConstColumnProxy<T, 2>(&m_data[col * 2]); }

        // Optimized element accessors
        constexpr T get(std::size_t row, std::size_t col) const noexcept { return m_data[col * 2 + row]; }
        constexpr T get(std::size_t index) const noexcept { return m_data[index]; }

        // Optimized element mutators
        constexpr void set(std::size_t row, std::size_t col, T value) noexcept { m_data[col * 2 + row] = value; }
        constexpr void set(std::size_t index, T value) noexcept { m_data[index] = value; }

        // Element access (row, column) - for backward compatibility
        constexpr T& operator()(std::size_t row, std::size_t col) noexcept { return m_data[col * 2 + row]; }
        constexpr const T& operator()(std::size_t row, std::size_t col) const noexcept { return m_data[col * 2 + row]; }

        // GLM-style column access functions
        constexpr Vector2D<T> column(std::size_t col) const noexcept { return Vector2D<T>(m_data[col * 2], m_data[col * 2 + 1]); }
        constexpr void setColumn(std::size_t col, const Vector2D<T>& column) noexcept
        {
            m_data[col * 2] = column.x;
            m_data[col * 2 + 1] = column.y;
        }

        // GLM-style row access functions
        constexpr Vector2D<T> row(std::size_t row) const noexcept { return Vector2D<T>(m_data[row], m_data[2 + row]); }

        // Size information
        constexpr std::size_t size() const noexcept { return 4; }

        // Compound assignment operators
        constexpr Matrix2x2& operator+=(const Matrix2x2& other) noexcept
        {
            for (std::size_t i = 0; i < 4; i++)
            {
                m_data[i] += other.m_data[i];
            }
            return *this;
        }

        constexpr Matrix2x2& operator-=(const Matrix2x2& other) noexcept
        {
            for (std::size_t i = 0; i < 4; i++)
            {
                m_data[i] -= other.m_data[i];
            }
            return *this;
        }

        constexpr Matrix2x2& operator*=(const Matrix2x2& other)
        {
            *this = *this * other;  // Uses non-member operator*
            return *this;
        }

        template <typename U>
        constexpr Matrix2x2& operator*=(U scalar) noexcept
        {
            for (auto& element : m_data)
            {
                element *= scalar;
            }
            return *this;
        }

        template <typename U>
        constexpr Matrix2x2& operator/=(U scalar) noexcept
        {
            for (auto& element : m_data)
            {
                element /= scalar;
            }
            return *this;
        }

        // Matrix operations
        constexpr T determinant() const noexcept
        {
            return m_data[0] * m_data[3] - m_data[1] * m_data[2];
        }

        constexpr Matrix2x2 transpose() const noexcept
        {
            return Matrix2x2(m_data[0], m_data[1], m_data[2], m_data[3]);
        }

        // Static factory methods
        static constexpr Matrix2x2 identity() noexcept
        {
            return Matrix2x2(T{1}, T{}, T{}, T{1});
        }

        static constexpr Matrix2x2 zero() noexcept
        {
            return Matrix2x2();
        }

    private:
        std::array<T, 4> m_data;

        // Friend declarations for non-member functions that need private access
        template <typename U> friend constexpr bool operator==(const Matrix2x2<U>& lhs, const Matrix2x2<U>& rhs);
        template <typename U> friend std::ostream& operator<<(std::ostream& os, const Matrix2x2<U>& mat);
    };

    template <typename T = float>
    class Matrix3x3
    {
    public:
        // Core constructors and Rule of 5
        constexpr Matrix3x3(T m00 = T{}, T m01 = T{}, T m02 = T{},
                            T m10 = T{}, T m11 = T{}, T m12 = T{},
                            T m20 = T{}, T m21 = T{}, T m22 = T{})
            : m_data{m00, m10, m20, m01, m11, m21, m02, m12, m22} {
        }
        constexpr Matrix3x3(const Vector3D<T>& col0, const Vector3D<T>& col1, const Vector3D<T>& col2)
            : m_data{col0.x, col0.y, col0.z, col1.x, col1.y, col1.z, col2.x, col2.y, col2.z} {
        }
        constexpr Matrix3x3(const Matrix3x3& other) = default;
        constexpr Matrix3x3(Matrix3x3&& other) noexcept = default;
        constexpr Matrix3x3& operator=(const Matrix3x3& other) = default;
        constexpr Matrix3x3& operator=(Matrix3x3&& other) noexcept = default;
        ~Matrix3x3() = default;

        // Template conversion constructor
        template <typename U>
        constexpr explicit Matrix3x3(const Matrix3x3<U>& other)
            : m_data{static_cast<T>(other.get(0)), static_cast<T>(other.get(1)), static_cast<T>(other.get(2)),
                     static_cast<T>(other.get(3)), static_cast<T>(other.get(4)), static_cast<T>(other.get(5)),
                     static_cast<T>(other.get(6)), static_cast<T>(other.get(7)), static_cast<T>(other.get(8))} {
        }

        // GLM-style column access: matrix[col][row]
        constexpr ColumnProxy<T, 3> operator[](std::size_t col) noexcept
        {
            return ColumnProxy<T, 3>(&m_data[col * 3]);
        }

        constexpr ConstColumnProxy<T, 3> operator[](std::size_t col) const noexcept
        {
            return ConstColumnProxy<T, 3>(&m_data[col * 3]);
        }

        // Optimized element accessors
        constexpr T get(std::size_t row, std::size_t col) const noexcept { return m_data[col * 3 + row]; }
        constexpr T get(std::size_t index) const noexcept { return m_data[index]; }

        // Optimized element mutators
        constexpr void set(std::size_t row, std::size_t col, T value) noexcept { m_data[col * 3 + row] = value; }
        constexpr void set(std::size_t index, T value) noexcept { m_data[index] = value; }

        // Element access (row, column) - for backward compatibility
        constexpr T& operator()(std::size_t row, std::size_t col) noexcept { return m_data[col * 3 + row]; }
        constexpr const T& operator()(std::size_t row, std::size_t col) const noexcept { return m_data[col * 3 + row]; }

        // GLM-style column access functions
        constexpr Vector3D<T> column(std::size_t col) const noexcept { return Vector3D<T>(m_data[col * 3], m_data[col * 3 + 1], m_data[col * 3 + 2]); }

        constexpr void setColumn(std::size_t col, const Vector3D<T>& column) noexcept
        {
            m_data[col * 3] = column.x;
            m_data[col * 3 + 1] = column.y;
            m_data[col * 3 + 2] = column.z;
        }

        // GLM-style row access functions
        constexpr Vector3D<T> row(std::size_t row) const noexcept { return Vector3D<T>(m_data[row], m_data[3 + row], m_data[6 + row]); }

        // Size information
        constexpr std::size_t size() const noexcept { return 9; }

        // Compound assignment operators
        constexpr Matrix3x3& operator+=(const Matrix3x3& other) noexcept
        {
            for (std::size_t i = 0; i < 9; i++)
            {
                m_data[i] += other.m_data[i];
            }
            return *this;
        }

        constexpr Matrix3x3& operator-=(const Matrix3x3& other) noexcept
        {
            for (std::size_t i = 0; i < 9; i++)
            {
                m_data[i] -= other.m_data[i];
            }
            return *this;
        }

        constexpr Matrix3x3& operator*=(const Matrix3x3& other)
        {
            *this = *this * other;  // Uses non-member operator*
            return *this;
        }

        template <typename U>
        constexpr Matrix3x3& operator*=(U scalar) noexcept
        {
            for (auto& element : m_data)
            {
                element *= scalar;
            }
            return *this;
        }

        template <typename U>
        constexpr Matrix3x3& operator/=(U scalar) noexcept
        {
            for (auto& element : m_data)
            {
                element /= scalar;
            }
            return *this;
        }

        // Matrix operations
        constexpr T determinant() const noexcept
        {
            return m_data[0] * (m_data[4] * m_data[8] - m_data[5] * m_data[7]) -
                   m_data[3] * (m_data[1] * m_data[8] - m_data[2] * m_data[7]) +
                   m_data[6] * (m_data[1] * m_data[5] - m_data[2] * m_data[4]);
        }

        constexpr Matrix3x3 transpose() const noexcept
        {
            return Matrix3x3(m_data[0], m_data[3], m_data[6],
                             m_data[1], m_data[4], m_data[7],
                             m_data[2], m_data[5], m_data[8]);
        }

        // Static factory methods
        static constexpr Matrix3x3 identity() noexcept
        {
            return Matrix3x3(T{1}, T{}, T{}, T{}, T{1}, T{}, T{}, T{}, T{1});
        }

        static constexpr Matrix3x3 zero() noexcept
        {
            return Matrix3x3();
        }

    private:
        std::array<T, 9> m_data;

        // Friend declarations for non-member functions that need private access
        template <typename U> friend constexpr bool operator==(const Matrix3x3<U>& lhs, const Matrix3x3<U>& rhs);
        template <typename U> friend std::ostream& operator<<(std::ostream& os, const Matrix3x3<U>& mat);
    };

    template <typename T = float>
    class Matrix4x4
    {
    public:
        // Core constructors and Rule of 5
        constexpr Matrix4x4(T m00 = T{}, T m01 = T{}, T m02 = T{}, T m03 = T{},
                            T m10 = T{}, T m11 = T{}, T m12 = T{}, T m13 = T{},
                            T m20 = T{}, T m21 = T{}, T m22 = T{}, T m23 = T{},
                            T m30 = T{}, T m31 = T{}, T m32 = T{}, T m33 = T{})
            : m_data{m00, m10, m20, m30, m01, m11, m21, m31, 
                     m02, m12, m22, m32, m03, m13, m23, m33} {
        }
        constexpr Matrix4x4(const Vector4D<T>& col0, const Vector4D<T>& col1, 
                            const Vector4D<T>& col2, const Vector4D<T>& col3)
            : m_data{col0.x, col0.y, col0.z, col0.w, col1.x, col1.y, col1.z, col1.w,
                     col2.x, col2.y, col2.z, col2.w, col3.x, col3.y, col3.z, col3.w} {
        }
        constexpr Matrix4x4(const Matrix4x4& other) = default;
        constexpr Matrix4x4(Matrix4x4&& other) noexcept = default;
        constexpr Matrix4x4& operator=(const Matrix4x4& other) = default;
        constexpr Matrix4x4& operator=(Matrix4x4&& other) noexcept = default;
        ~Matrix4x4() = default;

        // Template conversion constructor
        template <typename U>
        constexpr explicit Matrix4x4(const Matrix4x4<U>& other)
            : m_data{static_cast<T>(other.get(0)), static_cast<T>(other.get(1)), static_cast<T>(other.get(2)), static_cast<T>(other.get(3)),
                     static_cast<T>(other.get(4)), static_cast<T>(other.get(5)), static_cast<T>(other.get(6)), static_cast<T>(other.get(7)),
                     static_cast<T>(other.get(8)), static_cast<T>(other.get(9)), static_cast<T>(other.get(10)), static_cast<T>(other.get(11)),
                     static_cast<T>(other.get(12)), static_cast<T>(other.get(13)), static_cast<T>(other.get(14)), static_cast<T>(other.get(15))} {
        }

        // GLM-style column access: matrix[col][row]
        constexpr ColumnProxy<T, 4> operator[](std::size_t col) noexcept
        {
            return ColumnProxy<T, 4>(&m_data[col * 4]);
        }

        constexpr ConstColumnProxy<T, 4> operator[](std::size_t col) const noexcept
        {
            return ConstColumnProxy<T, 4>(&m_data[col * 4]);
        }

        // Optimized element accessors
        constexpr T get(std::size_t row, std::size_t col) const noexcept { return m_data[col * 4 + row]; }
        constexpr T get(std::size_t index) const noexcept { return m_data[index]; }

        // Optimized element mutators
        constexpr void set(std::size_t row, std::size_t col, T value) noexcept { m_data[col * 4 + row] = value; }
        constexpr void set(std::size_t index, T value) noexcept { m_data[index] = value; }

        // Element access (row, column) - for backward compatibility
        constexpr T& operator()(std::size_t row, std::size_t col) noexcept { return m_data[col * 4 + row]; }
        constexpr const T& operator()(std::size_t row, std::size_t col) const noexcept { return m_data[col * 4 + row]; }

        // GLM-style column access functions
        constexpr Vector4D<T> column(std::size_t col) const noexcept { return Vector4D<T>(m_data[col * 4], m_data[col * 4 + 1], m_data[col * 4 + 2], m_data[col * 4 + 3]); }

        constexpr void setColumn(std::size_t col, const Vector4D<T>& column) noexcept
        {
            m_data[col * 4] = column.x;
            m_data[col * 4 + 1] = column.y;
            m_data[col * 4 + 2] = column.z;
            m_data[col * 4 + 3] = column.w;
        }

        // GLM-style row access functions
        constexpr Vector4D<T> row(std::size_t row) const noexcept { return Vector4D<T>(m_data[row], m_data[4 + row], m_data[8 + row], m_data[12 + row]); }

        // Size information
        constexpr std::size_t size() const noexcept { return 16; }

        // Compound assignment operators
        constexpr Matrix4x4& operator+=(const Matrix4x4& other) noexcept
        {
            for (std::size_t i = 0; i < 16; i++)
            {
                m_data[i] += other.m_data[i];
            }
            return *this;
        }

        constexpr Matrix4x4& operator-=(const Matrix4x4& other) noexcept
        {
            for (std::size_t i = 0; i < 16; i++)
            {
                m_data[i] -= other.m_data[i];
            }
            return *this;
        }

        constexpr Matrix4x4& operator*=(const Matrix4x4& other)
        {
            *this = *this * other;  // Uses non-member operator*
            return *this;
        }

        template <typename U>
        constexpr Matrix4x4& operator*=(U scalar) noexcept
        {
            for (auto& element : m_data)
            {
                element *= scalar;
            }
            return *this;
        }

        template <typename U>
        constexpr Matrix4x4& operator/=(U scalar) noexcept
        {
            for (auto& element : m_data)
            {
                element /= scalar;
            }
            return *this;
        }

        // Matrix operations
        constexpr T determinant() const noexcept
        {
            return m_data[0] * (m_data[4] * m_data[8] - m_data[5] * m_data[7]) -
                    m_data[1] * (m_data[3] * m_data[8] - m_data[5] * m_data[6]) +
                    m_data[2] * (m_data[3] * m_data[7] - m_data[4] * m_data[6]);
        }

        constexpr Matrix4x4 transpose() const noexcept
        {
            return Matrix4x4(
                m_data[0], m_data[4], m_data[8], m_data[12],
                m_data[1], m_data[5], m_data[9], m_data[13],
                m_data[2], m_data[6], m_data[10], m_data[14],
                m_data[3], m_data[7], m_data[11], m_data[15]
            );
        }

        // Static factory methods
        static constexpr Matrix4x4 identity() noexcept
        {
            return Matrix4x4(T{1}, T{0}, T{0}, T{0}, T{0}, T{1}, T{0}, T{0}, T{0}, T{0}, T{1}, T{0}, T{0}, T{0}, T{0}, T{1});
        }

        static constexpr Matrix4x4 zero() noexcept
        {
            return Matrix4x4();
        }

    private:
        std::array<T, 16> m_data;

        // Friend declarations for non-member functions that need private access
        template <typename U> friend constexpr bool operator==(const Matrix4x4<U>& lhs, const Matrix4x4<U>& rhs);
        template <typename U> friend std::ostream& operator<<(std::ostream& os, const Matrix4x4<U>& mat);
    };

    // Arithmetic operators for Matrix2x2
    template <typename T>
    constexpr Matrix2x2<T> operator+(const Matrix2x2<T>& lhs, const Matrix2x2<T>& rhs)
    {
        return Matrix2x2<T>(lhs.get(0) + rhs.get(0), lhs.get(1) + rhs.get(1),
                            lhs.get(2) + rhs.get(2), lhs.get(3) + rhs.get(3));
    }

    template <typename T>
    constexpr Matrix2x2<T> operator-(const Matrix2x2<T>& lhs, const Matrix2x2<T>& rhs)
    {
        return Matrix2x2<T>(lhs.get(0) - rhs.get(0), lhs.get(1) - rhs.get(1),
                            lhs.get(2) - rhs.get(2), lhs.get(3) - rhs.get(3));
    }

    template <typename T, typename U>
    constexpr Matrix2x2<T> operator*(const Matrix2x2<T>& mat, U scalar)
    {
        return Matrix2x2<T>(mat.get(0) * scalar, mat.get(1) * scalar,
                            mat.get(2) * scalar, mat.get(3) * scalar);
    }

    template <typename T, typename U>
    constexpr Matrix2x2<T> operator*(U scalar, const Matrix2x2<T>& mat)
    {
        return mat * scalar;
    }

    template <typename T, typename U>
    constexpr Matrix2x2<T> operator/(const Matrix2x2<T>& mat, U scalar)
    {
        return Matrix2x2<T>(mat.get(0) / scalar, mat.get(1) / scalar,
                            mat.get(2) / scalar, mat.get(3) / scalar);
    }

    // Matrix multiplication for Matrix2x2
    template <typename T>
    constexpr Matrix2x2<T> operator*(const Matrix2x2<T>& lhs, const Matrix2x2<T>& rhs)
    {
        return Matrix2x2<T>(
            lhs.get(0, 0) * rhs.get(0, 0) + lhs.get(0, 1) * rhs.get(1, 0),
            lhs.get(0, 0) * rhs.get(0, 1) + lhs.get(0, 1) * rhs.get(1, 1),
            lhs.get(1, 0) * rhs.get(0, 0) + lhs.get(1, 1) * rhs.get(1, 0),
            lhs.get(1, 0) * rhs.get(0, 1) + lhs.get(1, 1) * rhs.get(1, 1)
        );
    }

    // Arithmetic operators for Matrix3x3
    template <typename T>
    constexpr Matrix3x3<T> operator+(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
    {
        Matrix3x3<T> result;
        for (std::size_t i = 0; i < 9; i++)
        {
            result.set(i, lhs.get(i) + rhs.get(i));
        }
        return result;
    }

    template <typename T>
    constexpr Matrix3x3<T> operator-(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
    {
        Matrix3x3<T> result;
        for (std::size_t i = 0; i < 9; i++)
        {
            result.set(i, lhs.get(i) - rhs.get(i));
        }
        return result;
    }

    template <typename T, typename U>
    constexpr Matrix3x3<T> operator*(const Matrix3x3<T>& mat, U scalar)
    {
        Matrix3x3<T> result;
        for (std::size_t i = 0; i < 9; i++)
        {
            result.set(i, mat.get(i) * scalar);
        }
        return result;
    }

    template <typename T, typename U>
    constexpr Matrix3x3<T> operator*(U scalar, const Matrix3x3<T>& mat)
    {
        return mat * scalar;
    }

    template <typename T, typename U>
    constexpr Matrix3x3<T> operator/(const Matrix3x3<T>& mat, U scalar)
    {
        Matrix3x3<T> result;
        for (std::size_t i = 0; i < 9; i++)
        {
            result.set(i, mat.get(i) / scalar);
        }
        return result;
    }

    // Matrix multiplication for Matrix3x3
    template <typename T>
    constexpr Matrix3x3<T> operator*(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
    {
        return Matrix3x3<T>(
            lhs.get(0, 0) * rhs.get(0, 0) + lhs.get(0, 1) * rhs.get(1, 0) + lhs.get(0, 2) * rhs.get(2, 0),
            lhs.get(0, 0) * rhs.get(0, 1) + lhs.get(0, 1) * rhs.get(1, 1) + lhs.get(0, 2) * rhs.get(2, 1),
            lhs.get(0, 0) * rhs.get(0, 2) + lhs.get(0, 1) * rhs.get(1, 2) + lhs.get(0, 2) * rhs.get(2, 2),
            lhs.get(1, 0) * rhs.get(0, 0) + lhs.get(1, 1) * rhs.get(1, 0) + lhs.get(1, 2) * rhs.get(2, 0),
            lhs.get(1, 0) * rhs.get(0, 1) + lhs.get(1, 1) * rhs.get(1, 1) + lhs.get(1, 2) * rhs.get(2, 1),
            lhs.get(1, 0) * rhs.get(0, 2) + lhs.get(1, 1) * rhs.get(1, 2) + lhs.get(1, 2) * rhs.get(2, 2),
            lhs.get(2, 0) * rhs.get(0, 0) + lhs.get(2, 1) * rhs.get(1, 0) + lhs.get(2, 2) * rhs.get(2, 0),
            lhs.get(2, 0) * rhs.get(0, 1) + lhs.get(2, 1) * rhs.get(1, 1) + lhs.get(2, 2) * rhs.get(2, 1),
            lhs.get(2, 0) * rhs.get(0, 2) + lhs.get(2, 1) * rhs.get(1, 2) + lhs.get(2, 2) * rhs.get(2, 2)
        );
    }

    // Arithmetic operators for Matrix4x4
    template <typename T>
    constexpr Matrix4x4<T> operator+(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs)
    {
        Matrix4x4<T> result;
        for (std::size_t i = 0; i < 16; i++)
        {
            result.set(i, lhs.get(i) + rhs.get(i));
        }
        return result;
    }

    template <typename T>
    constexpr Matrix4x4<T> operator-(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs)
    {
        Matrix4x4<T> result;
        for (std::size_t i = 0; i < 16; i++)
        {
            result.set(i, lhs.get(i) - rhs.get(i));
        }
        return result;
    }

    template <typename T, typename U>
    constexpr Matrix4x4<T> operator*(const Matrix4x4<T>& mat, U scalar)
    {
        Matrix4x4<T> result;
        for (std::size_t i = 0; i < 16; i++)
        {
            result.set(i, mat.get(i) * scalar);
        }
        return result;
    }

    template <typename T, typename U>
    constexpr Matrix4x4<T> operator*(U scalar, const Matrix4x4<T>& mat)
    {
        return mat * scalar;
    }

    template <typename T, typename U>
    constexpr Matrix4x4<T> operator/(const Matrix4x4<T>& mat, U scalar)
    {
        Matrix4x4<T> result;
        for (std::size_t i = 0; i < 16; i++)
        {
            result.set(i, mat.get(i) / scalar);
        }
        return result;
    }

    // Matrix multiplication for Matrix4x4
    template <typename T>
    constexpr Matrix4x4<T> operator*(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs)
    {
        return Matrix4x4<T>(
            lhs.get(0, 0) * rhs.get(0, 0) + lhs.get(0, 1) * rhs.get(1, 0) + lhs.get(0, 2) * rhs.get(2, 0) + lhs.get(0, 3) * rhs.get(3, 0),
            lhs.get(0, 0) * rhs.get(0, 1) + lhs.get(0, 1) * rhs.get(1, 1) + lhs.get(0, 2) * rhs.get(2, 1) + lhs.get(0, 3) * rhs.get(3, 1),
            lhs.get(0, 0) * rhs.get(0, 2) + lhs.get(0, 1) * rhs.get(1, 2) + lhs.get(0, 2) * rhs.get(2, 2) + lhs.get(0, 3) * rhs.get(3, 2),
            lhs.get(0, 0) * rhs.get(0, 3) + lhs.get(0, 1) * rhs.get(1, 3) + lhs.get(0, 2) * rhs.get(2, 3) + lhs.get(0, 3) * rhs.get(3, 3),

            lhs.get(1, 0) * rhs.get(0, 0) + lhs.get(1, 1) * rhs.get(1, 0) + lhs.get(1, 2) * rhs.get(2, 0) + lhs.get(1, 3) * rhs.get(3, 0),
            lhs.get(1, 0) * rhs.get(0, 1) + lhs.get(1, 1) * rhs.get(1, 1) + lhs.get(1, 2) * rhs.get(2, 1) + lhs.get(1, 3) * rhs.get(3, 1),
            lhs.get(1, 0) * rhs.get(0, 2) + lhs.get(1, 1) * rhs.get(1, 2) + lhs.get(1, 2) * rhs.get(2, 2) + lhs.get(1, 3) * rhs.get(3, 2),
            lhs.get(1, 0) * rhs.get(0, 3) + lhs.get(1, 1) * rhs.get(1, 3) + lhs.get(1, 2) * rhs.get(2, 3) + lhs.get(1, 3) * rhs.get(3, 3),

            lhs.get(2, 0) * rhs.get(0, 0) + lhs.get(2, 1) * rhs.get(1, 0) + lhs.get(2, 2) * rhs.get(2, 0) + lhs.get(2, 3) * rhs.get(3, 0),
            lhs.get(2, 0) * rhs.get(0, 1) + lhs.get(2, 1) * rhs.get(1, 1) + lhs.get(2, 2) * rhs.get(2, 1) + lhs.get(2, 3) * rhs.get(3, 1),
            lhs.get(2, 0) * rhs.get(0, 2) + lhs.get(2, 1) * rhs.get(1, 2) + lhs.get(2, 2) * rhs.get(2, 2) + lhs.get(2, 3) * rhs.get(3, 2),
            lhs.get(2, 0) * rhs.get(0, 3) + lhs.get(2, 1) * rhs.get(1, 3) + lhs.get(2, 2) * rhs.get(2, 3) + lhs.get(2, 3) * rhs.get(3, 3),

            lhs.get(3, 0) * rhs.get(0, 0) + lhs.get(3, 1) * rhs.get(1, 0) + lhs.get(3, 2) * rhs.get(2, 0) + lhs.get(3, 3) * rhs.get(3, 0),
            lhs.get(3, 0) * rhs.get(0, 1) + lhs.get(3, 1) * rhs.get(1, 1) + lhs.get(3, 2) * rhs.get(2, 1) + lhs.get(3, 3) * rhs.get(3, 1),
            lhs.get(3, 0) * rhs.get(0, 2) + lhs.get(3, 1) * rhs.get(1, 2) + lhs.get(3, 2) * rhs.get(2, 2) + lhs.get(3, 3) * rhs.get(3, 2),
            lhs.get(3, 0) * rhs.get(0, 3) + lhs.get(3, 1) * rhs.get(1, 3) + lhs.get(3, 2) * rhs.get(2, 3) + lhs.get(3, 3) * rhs.get(3, 3)
        );
    }

    // Unary minus operators
    template <typename T>
    constexpr Matrix2x2<T> operator-(const Matrix2x2<T>& mat)
    {
        return Matrix2x2<T>(-mat.get(0), -mat.get(1), -mat.get(2), -mat.get(3));
    }

    template <typename T>
    constexpr Matrix3x3<T> operator-(const Matrix3x3<T>& mat)
    {
        Matrix3x3<T> result;
        for (std::size_t i = 0; i < 9; i++)
        {
            result.set(i, -mat.get(i));
        }
        return result;
    }

    template <typename T>
    constexpr Matrix4x4<T> operator-(const Matrix4x4<T>& mat)
    {
        Matrix4x4<T> result;
        for (std::size_t i = 0; i < 16; i++)
        {
            result.set(i, -mat.get(i));
        }
        return result;
    }

    // Comparison operators
    template <typename T>
    constexpr bool operator==(const Matrix2x2<T>& lhs, const Matrix2x2<T>& rhs)
    {
        for (std::size_t i = 0; i < 4; i++)
        {
            if (lhs.m_data[i] != rhs.m_data[i]) return false;
        }
        return true;
    }

    template <typename T>
    constexpr bool operator!=(const Matrix2x2<T>& lhs, const Matrix2x2<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T>
    constexpr bool operator==(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
    {
        for (std::size_t i = 0; i < 9; i++)
        {
            if (lhs.m_data[i] != rhs.m_data[i]) return false;
        }
        return true;
    }

    template <typename T>
    constexpr bool operator!=(const Matrix3x3<T>& lhs, const Matrix3x3<T>& rhs)
    {
        return !(lhs == rhs);
    }

    template <typename T>
    constexpr bool operator==(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs)
    {
        for (std::size_t i = 0; i < 16; ++i) if (lhs.m_data[i] != rhs.m_data[i]) return false;
        return true;
    }
    template <typename T>
    constexpr bool operator!=(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs) { return !(lhs == rhs); }

    // Matrix-Vector multiplication
    template <typename T>
    constexpr Vector2D<T> operator*(const Matrix2x2<T>& mat, const Vector2D<T>& vec)
    {
        return Vector2D<T>(
            mat.get(0, 0) * vec.x() + mat.get(0, 1) * vec.y(),
            mat.get(1, 0) * vec.x() + mat.get(1, 1) * vec.y()
        );
    }

    template <typename T>
    constexpr Vector3D<T> operator*(const Matrix3x3<T>& mat, const Vector3D<T>& vec)
    {
        return Vector3D<T>(
            mat.get(0, 0) * vec.x() + mat.get(0, 1) * vec.y() + mat.get(0, 2) * vec.z(),
            mat.get(1, 0) * vec.x() + mat.get(1, 1) * vec.y() + mat.get(1, 2) * vec.z(),
            mat.get(2, 0) * vec.x() + mat.get(2, 1) * vec.y() + mat.get(2, 2) * vec.z()
        );
    }

    // Inverse operations (for floating point types)
    template <typename T>
    auto inverse(const Matrix2x2<T>& mat) -> std::enable_if_t<std::is_floating_point_v<T>, Matrix2x2<T>>
    {
        T det = mat.determinant();
        if (std::abs(det) < std::numeric_limits<T>::epsilon())
        {
            throw std::runtime_error("Matrix is not invertible, determinant is zero");
        }

        T inv_det = T{ 1 } / det;
        return Matrix2x2<T>(
            mat.get(1, 1) * inv_det, -mat.get(0, 1) * inv_det,
            -mat.get(1, 0) * inv_det, mat.get(0, 0) * inv_det
        );
    }

    template <typename T>
    auto inverse(const Matrix3x3<T>& mat) -> std::enable_if_t<std::is_floating_point_v<T>, Matrix3x3<T>>
    {
        T det = mat.determinant();
        if (std::abs(det) < std::numeric_limits<T>::epsilon())
        {
            throw std::runtime_error("Matrix is not invertible, determinant is zero");
        }

        T inv_det = T{ 1 } / det;
        return Matrix3x3<T>(
            (mat.get(1, 1) * mat.get(2, 2) - mat.get(1, 2) * mat.get(2, 1)) * inv_det,
            (mat.get(0, 2) * mat.get(2, 1) - mat.get(0, 1) * mat.get(2, 2)) * inv_det,
            (mat.get(0, 1) * mat.get(1, 2) - mat.get(0, 2) * mat.get(1, 1)) * inv_det,
            (mat.get(1, 2) * mat.get(2, 0) - mat.get(1, 0) * mat.get(2, 2)) * inv_det,
            (mat.get(0, 0) * mat.get(2, 2) - mat.get(0, 2) * mat.get(2, 0)) * inv_det,
            (mat.get(0, 2) * mat.get(1, 0) - mat.get(0, 0) * mat.get(1, 2)) * inv_det,
            (mat.get(1, 0) * mat.get(2, 1) - mat.get(1, 1) * mat.get(2, 0)) * inv_det,
            (mat.get(0, 1) * mat.get(2, 0) - mat.get(0, 0) * mat.get(2, 1)) * inv_det,
            (mat.get(0, 0) * mat.get(1, 1) - mat.get(0, 1) * mat.get(1, 0)) * inv_det
        );
    }

    // Stream operators
    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Matrix2x2<T>& mat)
    {
        os << "[" << mat.m_data[0] << ", " << mat.m_data[1] << "]\n";
        os << "[" << mat.m_data[2] << ", " << mat.m_data[3] << "]";
        return os;
    }

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Matrix3x3<T>& mat)
    {
        os << "[" << mat.m_data[0] << ", " << mat.m_data[1] << ", " << mat.m_data[2] << "]\n";
        os << "[" << mat.m_data[3] << ", " << mat.m_data[4] << ", " << mat.m_data[5] << "]\n";
        os << "[" << mat.m_data[6] << ", " << mat.m_data[7] << ", " << mat.m_data[8] << "]";
        return os;
    }

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Matrix4x4<T>& m)
    {
        os << "[" << m.m_data[0] << ", " << m.m_data[1] << ", " << m.m_data[2] << ", " << m.m_data[3] << "]\n";
        os << "[" << m.m_data[4] << ", " << m.m_data[5] << ", " << m.m_data[6] << ", " << m.m_data[7] << "]\n";
        os << "[" << m.m_data[8] << ", " << m.m_data[9] << ", " << m.m_data[10] << ", " << m.m_data[11] << "]\n";
        os << "[" << m.m_data[12] << ", " << m.m_data[13] << ", " << m.m_data[14] << ", " << m.m_data[15] << "]";
        return os;
    }
}
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Uma_Math
{
	template <typename T = float>
	class Vector2D
	{
	public:
		// Core constructors and Rule of 5
		constexpr Vector2D() : x{}, y{} {}
		constexpr Vector2D(T scalar) : x{scalar}, y{scalar} {}
		constexpr Vector2D(T x, T y) : x{x}, y{y} {}
		constexpr Vector2D(const Vector2D& other) = default;
		constexpr Vector2D(Vector2D&& other) noexcept = default;
		constexpr Vector2D& operator=(const Vector2D& other) = default;
		constexpr Vector2D& operator=(Vector2D&& other) noexcept = default;
		~Vector2D() = default;

		// Template conversion constructor
		template <typename U>
		constexpr explicit Vector2D(const Vector2D<U>& other) : x{ static_cast<T>(other.x) }, y{ static_cast<T>(other.y) } {}

        // GLM-style accessors
        constexpr T& x() noexcept { return x; }
        constexpr const T& x() const noexcept { return x; }
        constexpr T& y() noexcept { return y; }
        constexpr const T& y() const noexcept { return y; }

        // GLM-style color accessors
        constexpr T& r() noexcept { return x; }
        constexpr const T& r() const noexcept { return x; }
        constexpr T& g() noexcept { return y; }
        constexpr const T& g() const noexcept { return y; }

		// GLM-style texture coordinate accessors
        constexpr T& s() noexcept { return x; }
        constexpr const T& s() const noexcept { return x; }
        constexpr T& t() noexcept { return y; }
        constexpr const T& t() const noexcept { return y; }

		// Optimized mutators - inline and constexpr
		constexpr void setX(T x) noexcept { x = x; }
		constexpr void setY(T y) noexcept { y = y; }
		constexpr void set(T x, T y) noexcept { x = x; y = y; }

		// Array-style access
		constexpr T& operator[](std::size_t index)
		{
			return index == 0 ? x : y;
		}

		constexpr const T& operator[](std::size_t index) const
		{
			return index == 0 ? x : y;
		}

		// Size information
		constexpr std::size_t size() const noexcept { return 2; }

		// Compound assignment operators
		constexpr Vector2D& operator+=(const Vector2D& other) noexcept
		{
			x += other.x;
			y += other.y;
			return *this;
		}

		constexpr Vector2D& operator-=(const Vector2D& other) noexcept
		{
			x -= other.x;
			y -= other.y;
			return *this;
		}

        // Component-wise multiplication and division (GLM-style)
        constexpr Vector2D& operator*=(const Vector2D& other) noexcept
        {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        constexpr Vector2D& operator/=(const Vector2D& other) noexcept
        {
            x /= other.x;
            y /= other.y;
            return *this;
        }

		template <typename U>
		constexpr Vector2D& operator*=(U scalar) noexcept
		{
			x *= scalar;
			y *= scalar;
			return *this;
		}

		template <typename U>
		constexpr Vector2D& operator/=(U scalar) noexcept
		{
			x /= scalar;
			y /= scalar;
			return *this;
		}

		// Mutating operations
		void normalize()
		{
			auto mag = magnitude(*this);
			using MagType = decltype(mag);
			if (mag > MagType{})
			{
				if constexpr (std::is_integral_v<T>)
				{
					x = static_cast<T>(static_cast<double>(x) / mag);
					y = static_cast<T>(static_cast<double>(y) / mag);
				}
				else
				{
					x /= mag;
					y /= mag;
				}
			}
		}
	public:
		T x, y;
	};

	template <typename T = float>
	class Vector3D
	{
	public:
		// Core constructors and Rule of 5
		constexpr Vector3D() : x{}, y{}, z{} {}
        constexpr Vector3D(T scalar) : x{scalar}, y{scalar}, z{scalar} {}
        constexpr Vector3D(T x, T y, T z) : x{x}, y{y}, z{z} {}
        constexpr Vector3D(const Vector2D<T>& xy, T z) : x{xy.x}, y{xy.y}, z{z} {}
        constexpr Vector3D(T x, const Vector2D<T>& yz) : x{x}, y{yz.x}, z{yz.y} {}
        constexpr Vector3D(const Vector3D& other) = default;
        constexpr Vector3D(Vector3D&& other) noexcept = default;
        constexpr Vector3D& operator=(const Vector3D& other) = default;
        constexpr Vector3D& operator=(Vector3D&& other) noexcept = default;
        ~Vector3D() = default;

		// Template conversion constructor
		template <typename U>
		constexpr explicit Vector3D(const Vector3D<U>& other) : x{ static_cast<T>(other.x) }, y{ static_cast<T>(other.y) }, z{ static_cast<T>(other.z) } {}

        // GLM-style accessors
        constexpr T& x() noexcept { return x; }
        constexpr const T& x() const noexcept { return x; }
        constexpr T& y() noexcept { return y; }
        constexpr const T& y() const noexcept { return y; }
        constexpr T& z() noexcept { return z; }
        constexpr const T& z() const noexcept { return z; }
        
        // GLM-style color accessors
        constexpr T& r() noexcept { return x; }
        constexpr const T& r() const noexcept { return x; }
        constexpr T& g() noexcept { return y; }
        constexpr const T& g() const noexcept { return y; }
        constexpr T& b() noexcept { return z; }
        constexpr const T& b() const noexcept { return z; }
        
        // GLM-style texture coordinate accessors
        constexpr T& s() noexcept { return x; }
        constexpr const T& s() const noexcept { return x; }
        constexpr T& t() noexcept { return y; }
        constexpr const T& t() const noexcept { return y; }
        constexpr T& p() noexcept { return z; }
        constexpr const T& p() const noexcept { return z; }

        // GLM-style swizzle accessors (basic 2D swizzles)
        constexpr Vector2D<T> xy() const noexcept { return Vector2D<T>(x, y); }
        constexpr Vector2D<T> xz() const noexcept { return Vector2D<T>(x, z); }
        constexpr Vector2D<T> yz() const noexcept { return Vector2D<T>(y, z); }
        constexpr Vector2D<T> yx() const noexcept { return Vector2D<T>(y, x); }
        constexpr Vector2D<T> zx() const noexcept { return Vector2D<T>(z, x); }
        constexpr Vector2D<T> zy() const noexcept { return Vector2D<T>(z, y); }

		// Optimized mutators - inline and constexpr
		constexpr void setX(T x) noexcept { x = x; }
		constexpr void setY(T y) noexcept { y = y; }
		constexpr void setZ(T z) noexcept { z = z; }
		constexpr void set(T x, T y, T z) noexcept { x = x; y = y; z = z; }

		// Array-style access
		constexpr T& operator[](std::size_t index)
		{
			return index == 0 ? x : (index == 1 ? y : z);
		}

		constexpr const T& operator[](std::size_t index) const
		{
			return index == 0 ? x : (index == 1 ? y : z);
		}

		// Size information
		constexpr std::size_t size() const noexcept { return 3; }

		// Compound assignment operators
		constexpr Vector3D& operator+=(const Vector3D& other) noexcept
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}

		constexpr Vector3D& operator-=(const Vector3D& other) noexcept
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		constexpr Vector3D& operator*=(const Vector3D& other) noexcept
        {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            return *this;
        }

        constexpr Vector3D& operator/=(const Vector3D& other) noexcept
        {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            return *this;
        }

		template <typename U>
		constexpr Vector3D& operator*=(U scalar) noexcept
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return *this;
		}

		template <typename U>
		constexpr Vector3D& operator/=(U scalar) noexcept
		{
			x /= scalar;
			y /= scalar;
			z /= scalar;
			return *this;
		}

		// Mutating operations
		void normalize()
		{
			auto mag = magnitude(*this);
			using MagType = decltype(mag);
			if (mag > MagType{})
			{
				if constexpr (std::is_integral_v<T>)
				{
					x = static_cast<T>(static_cast<double>(x) / mag);
					y = static_cast<T>(static_cast<double>(y) / mag);
					z = static_cast<T>(static_cast<double>(z) / mag);
				}
				else
				{
					x /= mag;
					y /= mag;
					z /= mag;
				}
			}
		}
	public:
		T x, y, z;
	};

	// Arithmetic operators - using accessor functions
	template <typename T>
	constexpr Vector2D<T> operator+(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return Vector2D<T>(lhs.x + rhs.x, lhs.y + rhs.y);
	}

	template <typename T>
	constexpr Vector2D<T> operator-(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return Vector2D<T>(lhs.x - rhs.x, lhs.y - rhs.y);
	}

	template <typename T, typename U>
	constexpr Vector2D<T> operator*(const Vector2D<T>& vec, U scalar)
	{
		return Vector2D<T>(vec.x * scalar, vec.y * scalar);
	}

	template <typename T, typename U>
	constexpr Vector2D<T> operator*(U scalar, const Vector2D<T>& vec)
	{
		return vec * scalar;
	}

	template <typename T, typename U>
	constexpr Vector2D<T> operator/(const Vector2D<T>& vec, U scalar)
	{
		return Vector2D<T>(vec.x / scalar, vec.y / scalar);
	}

	template <typename T>
	constexpr Vector3D<T> operator+(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return Vector3D<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
	}

	template <typename T>
	constexpr Vector3D<T> operator-(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return Vector3D<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
	}

	template <typename T, typename U>
	constexpr Vector3D<T> operator*(const Vector3D<T>& vec, U scalar)
	{
		return Vector3D<T>(vec.x * scalar, vec.y * scalar, vec.z * scalar);
	}

	template <typename T, typename U>
	constexpr Vector3D<T> operator*(U scalar, const Vector3D<T>& vec)
	{
		return vec * scalar;
	}

	template <typename T, typename U>
	constexpr Vector3D<T> operator/(const Vector3D<T>& vec, U scalar)
	{
		return Vector3D<T>(vec.x / scalar, vec.y / scalar, vec.z / scalar);
	}

	// Unary minus operator
	template <typename T>
	constexpr Vector2D<T> operator-(const Vector2D<T>& vec)
	{
		return Vector2D<T>(-vec.x, -vec.y);
	}

	template <typename T>
	constexpr Vector3D<T> operator-(const Vector3D<T>& vec)
	{
		return Vector3D<T>(-vec.x, -vec.y, -vec.z);
	}

	// Comparison operators
	template <typename T>
	constexpr bool operator==(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}

	template <typename T>
	constexpr bool operator!=(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return !(lhs == rhs);
	}

	template <typename T>
	constexpr bool operator==(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
	}

	template <typename T>
	constexpr bool operator!=(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return !(lhs == rhs);
	}

	// Vector operations
	template <typename T>
	constexpr T dot(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}

	template <typename T>
	constexpr T dot(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	template <typename T>
	constexpr Vector3D<T> cross(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return Vector3D<T>(
			lhs.y * rhs.z - lhs.z * rhs.y,
			lhs.z * rhs.x - lhs.x * rhs.z,
			lhs.x * rhs.y - lhs.y * rhs.x
		);
	}

	// Magnitude operations
	template <typename T>
	constexpr T magnitudeSquared(const Vector2D<T>& vec)
	{
		return vec.x * vec.x + vec.y * vec.y;
	}

	template <typename T>
	constexpr T magnitudeSquared(const Vector3D<T>& vec)
	{
		return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
	}

	// Magnitude for floating point types
	template <typename T>
	auto magnitude(const Vector2D<T>& vec) -> std::enable_if_t<std::is_floating_point_v<T>, T>
	{
		return std::sqrt(magnitudeSquared(vec));
	}

	template <typename T>
	auto magnitude(const Vector3D<T>& vec) -> std::enable_if_t<std::is_floating_point_v<T>, T>
	{
		return std::sqrt(magnitudeSquared(vec));
	}

	// Magnitude for integer types (returns double)
	template <typename T>
	auto magnitude(const Vector2D<T>& vec) -> std::enable_if_t<std::is_integral_v<T>, double>
	{
		return std::sqrt(static_cast<double>(magnitudeSquared(vec)));
	}

	template <typename T>
	auto magnitude(const Vector3D<T>& vec) -> std::enable_if_t<std::is_integral_v<T>, double>
	{
		return std::sqrt(static_cast<double>(magnitudeSquared(vec)));
	}

	// Normalization for floating point types
	template <typename T>
	auto normalized(const Vector2D<T>& vec) -> std::enable_if_t<std::is_floating_point_v<T>, Vector2D<T>>
	{
		auto mag = magnitude(vec);
		if (mag > T{})
		{
			return vec / mag;
		}
		return Vector2D<T>(T{}, T{});
	}

	template <typename T>
	auto normalized(const Vector3D<T>& vec) -> std::enable_if_t<std::is_floating_point_v<T>, Vector3D<T>>
	{
		auto mag = magnitude(vec);
		if (mag > T{})
		{
			return vec / mag;
		}
		return Vector3D<T>(T{}, T{}, T{});
	}

	// Normalization for integer types (returns double vector)
	template <typename T>
	auto normalized(const Vector2D<T>& vec) -> std::enable_if_t<std::is_integral_v<T>, Vector2D<double>>
	{
		auto mag = magnitude(vec);
		if (mag > 0.0)
		{
			return Vector2D<double>(static_cast<double>(vec.x) / mag, static_cast<double>(vec.y) / mag);
		}
		return Vector2D<double>(0.0, 0.0);
	}

	template <typename T>
	auto normalized(const Vector3D<T>& vec) -> std::enable_if_t<std::is_integral_v<T>, Vector3D<double>>
	{
		auto mag = magnitude(vec);
		if (mag > 0.0)
		{
			return Vector3D<double>(static_cast<double>(vec.x) / mag, static_cast<double>(vec.y) / mag, static_cast<double>(vec.z) / mag);
		}
		return Vector3D<double>(0.0, 0.0, 0.0);
	}

	// Distance functions
	template <typename T>
	auto distance(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return magnitude(lhs - rhs);
	}

	template <typename T>
	auto distance(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return magnitude(lhs - rhs);
	}

	template <typename T>
	constexpr T distanceSquared(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return magnitudeSquared(lhs - rhs);
	}

	template <typename T>
	constexpr T distanceSquared(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return magnitudeSquared(lhs - rhs);
	}

	// Linear interpolation
	template <typename T, typename U>
	constexpr Vector2D<T> lerp(const Vector2D<T>& start, const Vector2D<T>& end, U t)
	{
		return start + (end - start) * t;
	}

	template <typename T, typename U>
	constexpr Vector3D<T> lerp(const Vector3D<T>& start, const Vector3D<T>& end, U t)
	{
		return start + (end - start) * t;
	}

	// Reflection
	template <typename T>
	constexpr Vector2D<T> reflect(const Vector2D<T>& incident, const Vector2D<T>& normal)
	{
		return incident - 2 * dot(incident, normal) * normal;
	}

	template <typename T>
	constexpr Vector3D<T> reflect(const Vector3D<T>& incident, const Vector3D<T>& normal)
	{
		return incident - 2 * dot(incident, normal) * normal;
	}

	// Angle between vectors (for floating point types)
	template <typename T>
	auto angle(const Vector2D<T>& lhs, const Vector2D<T>& rhs) -> std::enable_if_t<std::is_floating_point_v<T>, T>
	{
		auto dot_product = dot(lhs, rhs);
		auto magnitudes = magnitude(lhs) * magnitude(rhs);
		if (magnitudes > T{})
		{
			return std::acos(std::clamp(dot_product / magnitudes, T{ -1 }, T{ 1 }));
		}
		return T{};
	}

	template <typename T>
	auto angle(const Vector3D<T>& lhs, const Vector3D<T>& rhs) -> std::enable_if_t<std::is_floating_point_v<T>, T>
	{
		auto dot_product = dot(lhs, rhs);
		auto magnitudes = magnitude(lhs) * magnitude(rhs);
		if (magnitudes > T{})
		{
			return std::acos(std::clamp(dot_product / magnitudes, T{ -1 }, T{ 1 }));
		}
		return T{};
	}

	// Stream operators
	template <typename T>
	std::ostream& operator<<(std::ostream& os, const Vector2D<T>& vec)
	{
		os << "(" << vec.x << ", " << vec.y << ")";
		return os;
	}

	template <typename T>
	std::ostream& operator<<(std::ostream& os, const Vector3D<T>& vec)
	{
		os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
		return os;
	}
}
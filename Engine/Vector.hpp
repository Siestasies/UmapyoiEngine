#pragma once

#include <array>
#include <cmath>
#include <initializer_list>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Uma_Math
{
	template<typename T>
	class Vector2D
	{
	public:

		// Core constructors and Rule of 5
		constexpr Vector2D(T x = T{}, T y = T{}) : data{ x, y } {}
		constexpr Vector2D(const Vector2D& other) = default;
		constexpr Vector2D(Vector2D&& other) noexcept = default;
		constexpr Vector2D& operator=(const Vector2D& other) = default;
		constexpr Vector2D& operator=(Vector2D&& other) noexcept = default;
		~Vector2D() = default;

		// Template conversion constructor
		template<typename U>
		constexpr explicit Vector2D(const Vector2D<U>& other) : data{ static_cast<T>(other.x()), static_cast<T>(other.y()) } {}

		// Basic accessors
		constexpr T x() const { return data[0]; }
		constexpr T y() const { return data[1]; }
		constexpr void setX(T x) { data[0] = x; }
		constexpr void setY(T y) { data[1] = y; }

		// Array-style access
		constexpr T& operator[](std::size_t index) { return data[index]; }
		constexpr const T& operator[](std::size_t index) const { return data[index]; }

		// Size information
		constexpr std::size_t size() const { return data.size(); }

		// Compound assignment operators
		constexpr Vector2D& operator+=(const Vector2D& other)
		{
			data[0] += other.x();
			data[1] += other.y();
			return *this;
		}

		constexpr Vector2D& operator-=(const Vector2D& other)
		{
			data[0] -= other.x();
			data[1] -= other.y();
			return *this;
		}

		template<typename U>
		constexpr Vector2D& operator*=(U scalar)
		{
			data[0] *= scalar;
			data[1] *= scalar;
			return *this;
		}

		template<typename U>
		constexpr Vector2D& operator/=(U scalar)
		{
			data[0] /= scalar;
			data[1] /= scalar;
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
					data[0] = static_cast<T>(static_cast<double>(data[0]) / mag);
					data[1] = static_cast<T>(static_cast<double>(data[1]) / mag);
				}
				else
				{
					data[0] /= mag;
					data[1] /= mag;
				}
			}
		}

	private:
		std::array<T, 2> data;

	};

	template<typename T>
	class Vector3D
	{
	public:
		// Core constructors and Rule of 5
		constexpr Vector3D(T x = T{}, T y = T{}, T z = T{}) : data{ x, y, z } {}
		constexpr Vector3D(const Vector3D& other) = default;
		constexpr Vector3D(Vector3D&& other) noexcept = default;
		constexpr Vector3D& operator=(const Vector3D& other) = default;
		constexpr Vector3D& operator=(Vector3D&& other) noexcept = default;
		~Vector3D() = default;

		// Template conversion constructor
		template<typename U>
		constexpr explicit Vector3D(const Vector3D<U>& other) : data{ static_cast<T>(other.x()), static_cast<T>(other.y()), static_cast<T>(other.z()) } {}

		// Basic accessors
		constexpr T x() const { return data[0]; }
		constexpr T y() const { return data[1]; }
		constexpr T z() const { return data[2]; }
		constexpr void setX(T x) { data[0] = x; }
		constexpr void setY(T y) { data[1] = y; }
		constexpr void setZ(T z) { data[2] = z; }

		// Array-style access
		constexpr T& operator[](std::size_t index) { return data[index]; }
		constexpr const T& operator[](std::size_t index) const { return data[index]; }

		// Size information
		constexpr std::size_t size() const { return data.size(); }

		// Compound assignment operators
		constexpr Vector3D& operator+=(const Vector3D& other)
		{
			data[0] += other.x();
			data[1] += other.y();
			data[2] += other.z();
			return *this;
		}

		constexpr Vector3D& operator-=(const Vector3D& other)
		{
			data[0] -= other.x();
			data[1] -= other.y();
			data[2] -= other.z();
			return *this;
		}

		template<typename U>
		constexpr Vector3D& operator*=(U scalar)
		{
			data[0] *= scalar;
			data[1] *= scalar;
			data[2] *= scalar;
			return *this;
		}

		template<typename U>
		constexpr Vector3D& operator/=(U scalar)
		{
			data[0] /= scalar;
			data[1] /= scalar;
			data[2] /= scalar;
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
					data[0] = static_cast<T>(static_cast<double>(data[0]) / mag);
					data[1] = static_cast<T>(static_cast<double>(data[1]) / mag);
					data[2] = static_cast<T>(static_cast<double>(data[2]) / mag);
				}
				else
				{
					data[0] /= mag;
					data[1] /= mag;
					data[2] /= mag;
				}
			}
		}

	private:
		std::array<T, 3> data;
	};

	// Arithmetic operators
	template<typename T>
	constexpr Vector2D<T> operator+(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return Vector2D<T>(lhs.x() + rhs.x(), lhs.y() + rhs.y());
	}

	template<typename T>
	constexpr Vector2D<T> operator-(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return Vector2D<T>(lhs.x() - rhs.x(), lhs.y() - rhs.y());
	}

	template<typename T, typename U>
	constexpr Vector2D<T> operator*(const Vector2D<T>& vec, U scalar)
	{
		return Vector2D<T>(vec.x() * scalar, vec.y() * scalar);
	}

	template<typename T, typename U>
	constexpr Vector2D<T> operator*(U scalar, const Vector2D<T>& vec)
	{
		return vec * scalar;
	}

	template<typename T, typename U>
	constexpr Vector2D<T> operator/(const Vector2D<T>& vec, U scalar)
	{
		return Vector2D<T>(vec.x() / scalar, vec.y() / scalar);
	}

	template<typename T>
	constexpr Vector3D<T> operator+(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return Vector3D<T>(lhs.x() + rhs.x(), lhs.y() + rhs.y(), lhs.z() + rhs.z());
	}

	template<typename T>
	constexpr Vector3D<T> operator-(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return Vector3D<T>(lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z());
	}

	template<typename T, typename U>
	constexpr Vector3D<T> operator*(const Vector3D<T>& vec, U scalar)
	{
		return Vector3D<T>(vec.x() * scalar, vec.y() * scalar, vec.z() * scalar);
	}

	template<typename T, typename U>
	constexpr Vector3D<T> operator*(U scalar, const Vector3D<T>& vec)
	{
		return vec * scalar;
	}

	template<typename T, typename U>
	constexpr Vector3D<T> operator/(const Vector3D<T>& vec, U scalar)
	{
		return Vector3D<T>(vec.x() / scalar, vec.y() / scalar, vec.z() / scalar);
	}

	// Unary minus operator
	template<typename T>
	constexpr Vector2D<T> operator-(const Vector2D<T>& vec)
	{
		return Vector2D<T>(-vec.x(), -vec.y());
	}

	template<typename T>
	constexpr Vector3D<T> operator-(const Vector3D<T>& vec)
	{
		return Vector3D<T>(-vec.x(), -vec.y(), -vec.z());
	}

	// Comparison operators
	template<typename T>
	constexpr bool operator==(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return lhs.x() == rhs.x() && lhs.y() == rhs.y();
	}

	template<typename T>
	constexpr bool operator!=(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return !(lhs == rhs);
	}

	template<typename T>
	constexpr bool operator==(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return lhs.x() == rhs.x() && lhs.y() == rhs.y() && lhs.z() == rhs.z();
	}

	template<typename T>
	constexpr bool operator!=(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return !(lhs == rhs);
	}

	// Vector operations
	template<typename T>
	constexpr T dot(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return lhs.x() * rhs.x() + lhs.y() * rhs.y();
	}

	template<typename T>
	constexpr T dot(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return lhs.x() * rhs.x() + lhs.y() * rhs.y() + lhs.z() * rhs.z();
	}

	template<typename T>
	constexpr Vector3D<T> cross(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return Vector3D<T>(
			lhs.y() * rhs.z() - lhs.z() * rhs.y(),
			lhs.z() * rhs.x() - lhs.x() * rhs.z(),
			lhs.x() * rhs.y() - lhs.y() * rhs.x()
		);
	}

	// Magnitude operations
	template<typename T>
	constexpr T magnitudeSquared(const Vector2D<T>& vec)
	{
		return vec.x() * vec.x() + vec.y() * vec.y();
	}

	template<typename T>
	constexpr T magnitudeSquared(const Vector3D<T>& vec)
	{
		return vec.x() * vec.x() + vec.y() * vec.y() + vec.z() * vec.z();
	}

	// Magnitude for floating point types
	template<typename T>
	auto magnitude(const Vector2D<T>& vec) -> std::enable_if_t<std::is_floating_point_v<T>, T>
	{
		return std::sqrt(magnitudeSquared(vec));
	}

	template<typename T>
	auto magnitude(const Vector3D<T>& vec) -> std::enable_if_t<std::is_floating_point_v<T>, T>
	{
		return std::sqrt(magnitudeSquared(vec));
	}

	// Magnitude for integer types (returns double)
	template<typename T>
	auto magnitude(const Vector2D<T>& vec) -> std::enable_if_t<std::is_integral_v<T>, double>
	{
		return std::sqrt(static_cast<double>(magnitudeSquared(vec)));
	}

	template<typename T>
	auto magnitude(const Vector3D<T>& vec) -> std::enable_if_t<std::is_integral_v<T>, double>
	{
		return std::sqrt(static_cast<double>(magnitudeSquared(vec)));
	}

	// Normalization for floating point types
	template<typename T>
	auto normalized(const Vector2D<T>& vec) -> std::enable_if_t<std::is_floating_point_v<T>, Vector2D<T>>
	{
		auto mag = magnitude(vec);
		if (mag > T{})
		{
			return vec / mag;
		}
		return Vector2D<T>(T{}, T{});
	}

	template<typename T>
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
	template<typename T>
	auto normalized(const Vector2D<T>& vec) -> std::enable_if_t<std::is_integral_v<T>, Vector2D<double>>
	{
		auto mag = magnitude(vec);
		if (mag > 0.0)
		{
			return Vector2D<double>(static_cast<double>(vec.x()) / mag,
				static_cast<double>(vec.y()) / mag);
		}
		return Vector2D<double>(0.0, 0.0);
	}

	template<typename T>
	auto normalized(const Vector3D<T>& vec) -> std::enable_if_t<std::is_integral_v<T>, Vector3D<double>>
	{
		auto mag = magnitude(vec);
		if (mag > 0.0)
		{
			return Vector3D<double>(static_cast<double>(vec.x()) / mag,
				static_cast<double>(vec.y()) / mag,
				static_cast<double>(vec.z()) / mag);
		}
		return Vector3D<double>(0.0, 0.0, 0.0);
	}

	// Distance functions
	template<typename T>
	auto distance(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return magnitude(lhs - rhs);
	}

	template<typename T>
	auto distance(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return magnitude(lhs - rhs);
	}

	template<typename T>
	constexpr T distanceSquared(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
	{
		return magnitudeSquared(lhs - rhs);
	}

	template<typename T>
	constexpr T distanceSquared(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
	{
		return magnitudeSquared(lhs - rhs);
	}

	// Linear interpolation
	template<typename T, typename U>
	constexpr Vector2D<T> lerp(const Vector2D<T>& start, const Vector2D<T>& end, U t)
	{
		return start + (end - start) * t;
	}

	template<typename T, typename U>
	constexpr Vector3D<T> lerp(const Vector3D<T>& start, const Vector3D<T>& end, U t)
	{
		return start + (end - start) * t;
	}

	// Reflection
	template<typename T>
	constexpr Vector2D<T> reflect(const Vector2D<T>& incident, const Vector2D<T>& normal)
	{
		return incident - 2 * dot(incident, normal) * normal;
	}

	template<typename T>
	constexpr Vector3D<T> reflect(const Vector3D<T>& incident, const Vector3D<T>& normal)
	{
		return incident - 2 * dot(incident, normal) * normal;
	}

	// Angle between vectors (for floating point types)
	template<typename T>
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

	template<typename T>
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
	template<typename T>
	std::ostream& operator<<(std::ostream& os, const Vector2D<T>& vec)
	{
		os << "(" << vec.x() << ", " << vec.y() << ")";
		return os;
	}

	template<typename T>
	std::ostream& operator<<(std::ostream& os, const Vector3D<T>& vec)
	{
		os << "(" << vec.x() << ", " << vec.y() << ", " << vec.z() << ")";
		return os;
	}

	// Compile-time constants for different types
	namespace constants
	{
		constexpr Vector2D<float> UNIT_X_2F(1.0f, 0.0f);
		constexpr Vector2D<float> UNIT_Y_2F(0.0f, 1.0f);
		constexpr Vector2D<float> ZERO_2F(0.0f, 0.0f);

		constexpr Vector2D<double> UNIT_X_2D(1.0, 0.0);
		constexpr Vector2D<double> UNIT_Y_2D(0.0, 1.0);
		constexpr Vector2D<double> ZERO_2D(0.0, 0.0);

		constexpr Vector2D<int> UNIT_X_2I(1, 0);
		constexpr Vector2D<int> UNIT_Y_2I(0, 1);
		constexpr Vector2D<int> ZERO_2I(0, 0);

		constexpr Vector3D<float> UNIT_X_3F(1.0f, 0.0f, 0.0f);
		constexpr Vector3D<float> UNIT_Y_3F(0.0f, 1.0f, 0.0f);
		constexpr Vector3D<float> UNIT_Z_3F(0.0f, 0.0f, 1.0f);
		constexpr Vector3D<float> ZERO_3F(0.0f, 0.0f, 0.0f);

		constexpr Vector3D<double> UNIT_X_3D(1.0, 0.0, 0.0);
		constexpr Vector3D<double> UNIT_Y_3D(0.0, 1.0, 0.0);
		constexpr Vector3D<double> UNIT_Z_3D(0.0, 0.0, 1.0);
		constexpr Vector3D<double> ZERO_3D(0.0, 0.0, 0.0);

		constexpr Vector3D<int> UNIT_X_3I(1, 0, 0);
		constexpr Vector3D<int> UNIT_Y_3I(0, 1, 0);
		constexpr Vector3D<int> UNIT_Z_3I(0, 0, 1);
		constexpr Vector3D<int> ZERO_3I(0, 0, 0);
	}
}
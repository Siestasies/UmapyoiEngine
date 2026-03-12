/*!
\file   Vector.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Defines templated 2D and 3D vector classes supporting arithmetic,
vector operations, and common utilities for both integral and floating-point types.

Provides GLM-style accessors, swizzle functions (3D), and overloaded operators
for intuitive usage. Includes functions for magnitude, normalization,
dot and cross products, distance, reflection, interpolation, and angle calculations.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

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
		/*!
		 * \class Vector2D
		 * \brief Templated 2D vector class supporting arithmetic and vector operations.
		 */
		template <typename T = float>
		class Vector2D
		{
		public:
				/*! \brief Default constructor. Initializes components to zero. */
				constexpr Vector2D() : x{}, y{} {}
				/*! \brief Constructs a vector with both components set to the same scalar. \param scalar Value for both x and y. */
				constexpr Vector2D(T scalar) : x{ scalar }, y{ scalar } {}
				/*! \brief Constructs a vector from x and y components. \param x X component. \param y Y component. */
				constexpr Vector2D(T x, T y) : x{ x }, y{ y } {}
				constexpr Vector2D(const Vector2D& other) = default;
				constexpr Vector2D(Vector2D&& other) noexcept = default;
				constexpr Vector2D& operator=(const Vector2D& other) = default;
				constexpr Vector2D& operator=(Vector2D&& other) noexcept = default;
				~Vector2D() = default;

				/*! \brief Converts from a Vector2D of a different type. \param other Source vector to convert from. */
				template <typename U>
				constexpr explicit Vector2D(const Vector2D<U>& other) : x{ static_cast<T>(other.x) }, y{ static_cast<T>(other.y) } {}

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

				/*! \brief Sets the X component. \param x New X value. */
				constexpr void setX(T x) noexcept { x = x; }
				/*! \brief Sets the Y component. \param y New Y value. */
				constexpr void setY(T y) noexcept { y = y; }
				/*! \brief Sets both components. \param x New X value. \param y New Y value. */
				constexpr void set(T x, T y) noexcept { x = x; y = y; }

				/*!
				 * \brief Array-style element access.
				 * \param index Component index (0=x, 1=y).
				 * \return Reference to the component.
				 */
				constexpr T& operator[](std::size_t index)
				{
						return index == 0 ? x : y;
				}

				/*! \brief Const array-style element access. \param index Component index (0=x, 1=y). \return Const reference to the component. */
				constexpr const T& operator[](std::size_t index) const
				{
						return index == 0 ? x : y;
				}

				/*! \brief Returns the number of components (2). \return Always 2. */
				constexpr std::size_t size() const noexcept { return 2; }

				/*! \brief Adds another vector to this one. \param other Vector to add. \return Reference to this vector. */
				constexpr Vector2D& operator+=(const Vector2D& other) noexcept
				{
						x += other.x;
						y += other.y;
						return *this;
				}

				/*! \brief Subtracts another vector from this one. \param other Vector to subtract. \return Reference to this vector. */
				constexpr Vector2D& operator-=(const Vector2D& other) noexcept
				{
						x -= other.x;
						y -= other.y;
						return *this;
				}

				/*! \brief Component-wise multiplies by another vector. \param other Vector to multiply by. \return Reference to this vector. */
				constexpr Vector2D& operator*=(const Vector2D& other) noexcept
				{
						x *= other.x;
						y *= other.y;
						return *this;
				}

				/*! \brief Component-wise divides by another vector. \param other Vector to divide by. \return Reference to this vector. */
				constexpr Vector2D& operator/=(const Vector2D& other) noexcept
				{
						x /= other.x;
						y /= other.y;
						return *this;
				}

				/*! \brief Multiplies all components by a scalar. \param scalar Value to multiply by. \return Reference to this vector. */
				template <typename U>
				constexpr Vector2D& operator*=(U scalar) noexcept
				{
						x *= scalar;
						y *= scalar;
						return *this;
				}

				/*! \brief Divides all components by a scalar. \param scalar Value to divide by. \return Reference to this vector. */
				template <typename U>
				constexpr Vector2D& operator/=(U scalar) noexcept
				{
						x /= scalar;
						y /= scalar;
						return *this;
				}

				/*! \brief Normalizes this vector in-place to unit length. */
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

		/*!
		 * \class Vector3D
		 * \brief Templated 3D vector class supporting arithmetic, cross product, and swizzle operations.
		 */
		template <typename T = float>
		class Vector3D
		{
		public:
				/*! \brief Default constructor. Initializes components to zero. */
				constexpr Vector3D() : x{}, y{}, z{} {}
				/*! \brief Constructs a vector with all components set to the same scalar. \param scalar Value for x, y, and z. */
				constexpr Vector3D(T scalar) : x{ scalar }, y{ scalar }, z{ scalar } {}
				/*! \brief Constructs a vector from x, y, and z components. \param x X component. \param y Y component. \param z Z component. */
				constexpr Vector3D(T x, T y, T z) : x{ x }, y{ y }, z{ z } {}
				/*! \brief Constructs from a 2D vector (xy) and a z component. \param xy XY components. \param z Z component. */
				constexpr Vector3D(const Vector2D<T>& xy, T z) : x{ xy.x }, y{ xy.y }, z{ z } {}
				/*! \brief Constructs from an x component and a 2D vector (yz). \param x X component. \param yz YZ components. */
				constexpr Vector3D(T x, const Vector2D<T>& yz) : x{ x }, y{ yz.x }, z{ yz.y } {}
				constexpr Vector3D(const Vector3D& other) = default;
				constexpr Vector3D(Vector3D&& other) noexcept = default;
				constexpr Vector3D& operator=(const Vector3D& other) = default;
				constexpr Vector3D& operator=(Vector3D&& other) noexcept = default;
				~Vector3D() = default;

				/*! \brief Converts from a Vector3D of a different type. \param other Source vector to convert from. */
				template <typename U>
				constexpr explicit Vector3D(const Vector3D<U>& other) : x{ static_cast<T>(other.x) }, y{ static_cast<T>(other.y) }, z{ static_cast<T>(other.z) } {}

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

				/*! \brief Sets the X component. \param x New X value. */
				constexpr void setX(T x) noexcept { x = x; }
				/*! \brief Sets the Y component. \param y New Y value. */
				constexpr void setY(T y) noexcept { y = y; }
				/*! \brief Sets the Z component. \param z New Z value. */
				constexpr void setZ(T z) noexcept { z = z; }
				/*! \brief Sets all three components. \param x New X value. \param y New Y value. \param z New Z value. */
				constexpr void set(T x, T y, T z) noexcept { x = x; y = y; z = z; }

				/*! \brief Array-style element access. \param index Component index (0=x, 1=y, 2=z). \return Reference to the component. */
				constexpr T& operator[](std::size_t index)
				{
						return index == 0 ? x : (index == 1 ? y : z);
				}

				/*! \brief Const array-style element access. \param index Component index (0=x, 1=y, 2=z). \return Const reference to the component. */
				constexpr const T& operator[](std::size_t index) const
				{
						return index == 0 ? x : (index == 1 ? y : z);
				}

				/*! \brief Returns the number of components (3). \return Always 3. */
				constexpr std::size_t size() const noexcept { return 3; }

				/*! \brief Adds another vector to this one. \param other Vector to add. \return Reference to this vector. */
				constexpr Vector3D& operator+=(const Vector3D& other) noexcept
				{
						x += other.x;
						y += other.y;
						z += other.z;
						return *this;
				}

				/*! \brief Subtracts another vector from this one. \param other Vector to subtract. \return Reference to this vector. */
				constexpr Vector3D& operator-=(const Vector3D& other) noexcept
				{
						x -= other.x;
						y -= other.y;
						z -= other.z;
						return *this;
				}

				/*! \brief Component-wise multiplies by another vector. \param other Vector to multiply by. \return Reference to this vector. */
				constexpr Vector3D& operator*=(const Vector3D& other) noexcept
				{
						x *= other.x;
						y *= other.y;
						z *= other.z;
						return *this;
				}

				/*! \brief Component-wise divides by another vector. \param other Vector to divide by. \return Reference to this vector. */
				constexpr Vector3D& operator/=(const Vector3D& other) noexcept
				{
						x /= other.x;
						y /= other.y;
						z /= other.z;
						return *this;
				}

				/*! \brief Multiplies all components by a scalar. \param scalar Value to multiply by. \return Reference to this vector. */
				template <typename U>
				constexpr Vector3D& operator*=(U scalar) noexcept
				{
						x *= scalar;
						y *= scalar;
						z *= scalar;
						return *this;
				}

				/*! \brief Divides all components by a scalar. \param scalar Value to divide by. \return Reference to this vector. */
				template <typename U>
				constexpr Vector3D& operator/=(U scalar) noexcept
				{
						x /= scalar;
						y /= scalar;
						z /= scalar;
						return *this;
				}

				/*! \brief Normalizes this vector in-place to unit length. */
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

		/*! \brief Adds two 2D vectors. \param lhs Left operand. \param rhs Right operand. \return Component-wise sum. */
		template <typename T>
		constexpr Vector2D<T> operator+(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
		{
				return Vector2D<T>(lhs.x + rhs.x, lhs.y + rhs.y);
		}

		/*! \brief Subtracts two 2D vectors. \param lhs Left operand. \param rhs Right operand. \return Component-wise difference. */
		template <typename T>
		constexpr Vector2D<T> operator-(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
		{
				return Vector2D<T>(lhs.x - rhs.x, lhs.y - rhs.y);
		}

		/*! \brief Component-wise multiplies two 2D vectors. \param lhs Left operand. \param rhs Right operand. \return Component-wise product. */
		template <typename T>
		constexpr Vector2D<T> operator*(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
		{
				return Vector2D<T>(lhs.x * rhs.x, lhs.y * rhs.y);
		}

		/*! \brief Component-wise divides two 2D vectors. \param lhs Left operand. \param rhs Right operand. \return Component-wise quotient. */
		template <typename T>
		constexpr Vector2D<T> operator/(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
		{
				return Vector2D<T>(lhs.x / rhs.x, lhs.y / rhs.y);
		}

		/*! \brief Multiplies a 2D vector by a scalar. \param vec The vector. \param scalar The scalar. \return Scaled vector. */
		template <typename T, typename U>
		constexpr Vector2D<T> operator*(const Vector2D<T>& vec, U scalar)
		{
				return Vector2D<T>(vec.x * scalar, vec.y * scalar);
		}

		/*! \brief Multiplies a scalar by a 2D vector. \param scalar The scalar. \param vec The vector. \return Scaled vector. */
		template <typename T, typename U>
		constexpr Vector2D<T> operator*(U scalar, const Vector2D<T>& vec)
		{
				return vec * scalar;
		}

		/*! \brief Divides a 2D vector by a scalar. \param vec The vector. \param scalar The scalar. \return Scaled vector. */
		template <typename T, typename U>
		constexpr Vector2D<T> operator/(const Vector2D<T>& vec, U scalar)
		{
				return Vector2D<T>(vec.x / scalar, vec.y / scalar);
		}

		/*! \brief Divides a scalar by each component of a 2D vector. \param scalar The scalar. \param vec The vector. \return Component-wise quotient. */
		template <typename T, typename U>
		constexpr Vector2D<T> operator/(U scalar, const Vector2D<T>& vec)
		{
				return Vector2D<T>(scalar / vec.x, scalar / vec.y);
		}

		/*! \brief Adds two 3D vectors. \param lhs Left operand. \param rhs Right operand. \return Component-wise sum. */
		template <typename T>
		constexpr Vector3D<T> operator+(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
		{
				return Vector3D<T>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
		}

		/*! \brief Subtracts two 3D vectors. \param lhs Left operand. \param rhs Right operand. \return Component-wise difference. */
		template <typename T>
		constexpr Vector3D<T> operator-(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
		{
				return Vector3D<T>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
		}

		/*! \brief Component-wise multiplies two 3D vectors. \param lhs Left operand. \param rhs Right operand. \return Component-wise product. */
		template <typename T>
		constexpr Vector3D<T> operator*(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
		{
				return Vector3D<T>(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
		}

		/*! \brief Component-wise divides two 3D vectors. \param lhs Left operand. \param rhs Right operand. \return Component-wise quotient. */
		template <typename T>
		constexpr Vector3D<T> operator/(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
		{
				return Vector3D<T>(lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z);
		}

		/*! \brief Multiplies a 3D vector by a scalar. \param vec The vector. \param scalar The scalar. \return Scaled vector. */
		template <typename T, typename U>
		constexpr Vector3D<T> operator*(const Vector3D<T>& vec, U scalar)
		{
				return Vector3D<T>(vec.x * scalar, vec.y * scalar, vec.z * scalar);
		}

		/*! \brief Multiplies a scalar by a 3D vector. \param scalar The scalar. \param vec The vector. \return Scaled vector. */
		template <typename T, typename U>
		constexpr Vector3D<T> operator*(U scalar, const Vector3D<T>& vec)
		{
				return vec * scalar;
		}

		/*! \brief Divides a 3D vector by a scalar. \param vec The vector. \param scalar The scalar. \return Scaled vector. */
		template <typename T, typename U>
		constexpr Vector3D<T> operator/(const Vector3D<T>& vec, U scalar)
		{
				return Vector3D<T>(vec.x / scalar, vec.y / scalar, vec.z / scalar);
		}

		/*! \brief Divides a scalar by each component of a 3D vector. \param scalar The scalar. \param vec The vector. \return Component-wise quotient. */
		template <typename T, typename U>
		constexpr Vector3D<T> operator/(U scalar, const Vector3D<T>& vec)
		{
				return Vector3D<T>(scalar / vec.x, scalar / vec.y, scalar / vec.z);
		}

		/*! \brief Negates a 2D vector. \param vec The vector to negate. \return Negated vector. */
		template <typename T>
		constexpr Vector2D<T> operator-(const Vector2D<T>& vec)
		{
				return Vector2D<T>(-vec.x, -vec.y);
		}

		/*! \brief Negates a 3D vector. \param vec The vector to negate. \return Negated vector. */
		template <typename T>
		constexpr Vector3D<T> operator-(const Vector3D<T>& vec)
		{
				return Vector3D<T>(-vec.x, -vec.y, -vec.z);
		}

		/*! \brief Tests equality of two 2D vectors. \param lhs Left operand. \param rhs Right operand. \return True if all components are equal. */
		template <typename T>
		constexpr bool operator==(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
		{
				return lhs.x == rhs.x && lhs.y == rhs.y;
		}

		/*! \brief Tests inequality of two 2D vectors. \param lhs Left operand. \param rhs Right operand. \return True if any component differs. */
		template <typename T>
		constexpr bool operator!=(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
		{
				return !(lhs == rhs);
		}

		/*! \brief Tests equality of two 3D vectors. \param lhs Left operand. \param rhs Right operand. \return True if all components are equal. */
		template <typename T>
		constexpr bool operator==(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
		{
				return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
		}

		/*! \brief Tests inequality of two 3D vectors. \param lhs Left operand. \param rhs Right operand. \return True if any component differs. */
		template <typename T>
		constexpr bool operator!=(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
		{
				return !(lhs == rhs);
		}

		/*! \brief Computes the dot product of two 2D vectors. \param lhs Left operand. \param rhs Right operand. \return Dot product scalar. */
		template <typename T>
		constexpr T dot(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
		{
				return lhs.x * rhs.x + lhs.y * rhs.y;
		}

		/*! \brief Computes the dot product of two 3D vectors. \param lhs Left operand. \param rhs Right operand. \return Dot product scalar. */
		template <typename T>
		constexpr T dot(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
		{
				return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
		}

		/*! \brief Computes the cross product of two 3D vectors. \param lhs Left operand. \param rhs Right operand. \return Cross product vector. */
		template <typename T>
		constexpr Vector3D<T> cross(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
		{
				return Vector3D<T>(
						lhs.y * rhs.z - lhs.z * rhs.y,
						lhs.z * rhs.x - lhs.x * rhs.z,
						lhs.x * rhs.y - lhs.y * rhs.x
				);
		}

		/*! \brief Computes the squared magnitude of a 2D vector. \param vec The vector. \return Squared magnitude. */
		template <typename T>
		constexpr T magnitudeSquared(const Vector2D<T>& vec)
		{
				return vec.x * vec.x + vec.y * vec.y;
		}

		/*! \brief Computes the squared magnitude of a 3D vector. \param vec The vector. \return Squared magnitude. */
		template <typename T>
		constexpr T magnitudeSquared(const Vector3D<T>& vec)
		{
				return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
		}

		/*! \brief Computes the magnitude of a 2D floating-point vector. \param vec The vector. \return Magnitude. */
		template <typename T>
		auto magnitude(const Vector2D<T>& vec) -> std::enable_if_t<std::is_floating_point_v<T>, T>
		{
				return std::sqrt(magnitudeSquared(vec));
		}

		/*! \brief Computes the magnitude of a 3D floating-point vector. \param vec The vector. \return Magnitude. */
		template <typename T>
		auto magnitude(const Vector3D<T>& vec) -> std::enable_if_t<std::is_floating_point_v<T>, T>
		{
				return std::sqrt(magnitudeSquared(vec));
		}

		/*! \brief Computes the magnitude of a 2D integer vector (returns double). \param vec The vector. \return Magnitude as double. */
		template <typename T>
		auto magnitude(const Vector2D<T>& vec) -> std::enable_if_t<std::is_integral_v<T>, double>
		{
				return std::sqrt(static_cast<double>(magnitudeSquared(vec)));
		}

		/*! \brief Computes the magnitude of a 3D integer vector (returns double). \param vec The vector. \return Magnitude as double. */
		template <typename T>
		auto magnitude(const Vector3D<T>& vec) -> std::enable_if_t<std::is_integral_v<T>, double>
		{
				return std::sqrt(static_cast<double>(magnitudeSquared(vec)));
		}

		/*! \brief Returns a normalized copy of a 2D floating-point vector. \param vec The vector to normalize. \return Unit-length vector, or zero vector if magnitude is zero. */
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

		/*! \brief Returns a normalized copy of a 3D floating-point vector. \param vec The vector to normalize. \return Unit-length vector, or zero vector if magnitude is zero. */
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

		/*! \brief Returns a normalized copy of a 2D integer vector as double. \param vec The vector to normalize. \return Unit-length double vector, or zero vector if magnitude is zero. */
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

		/*! \brief Returns a normalized copy of a 3D integer vector as double. \param vec The vector to normalize. \return Unit-length double vector, or zero vector if magnitude is zero. */
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

		/*! \brief Computes the Euclidean distance between two 2D vectors. \param lhs First point. \param rhs Second point. \return Distance between the two points. */
		template <typename T>
		auto distance(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
		{
				return magnitude(lhs - rhs);
		}

		/*! \brief Computes the Euclidean distance between two 3D vectors. \param lhs First point. \param rhs Second point. \return Distance between the two points. */
		template <typename T>
		auto distance(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
		{
				return magnitude(lhs - rhs);
		}

		/*! \brief Computes the squared distance between two 2D vectors. \param lhs First point. \param rhs Second point. \return Squared distance. */
		template <typename T>
		constexpr T distanceSquared(const Vector2D<T>& lhs, const Vector2D<T>& rhs)
		{
				return magnitudeSquared(lhs - rhs);
		}

		/*! \brief Computes the squared distance between two 3D vectors. \param lhs First point. \param rhs Second point. \return Squared distance. */
		template <typename T>
		constexpr T distanceSquared(const Vector3D<T>& lhs, const Vector3D<T>& rhs)
		{
				return magnitudeSquared(lhs - rhs);
		}

		/*! \brief Linearly interpolates between two 2D vectors. \param start Start vector. \param end End vector. \param t Interpolation factor [0,1]. \return Interpolated vector. */
		template <typename T, typename U>
		constexpr Vector2D<T> lerp(const Vector2D<T>& start, const Vector2D<T>& end, U t)
		{
				return start + (end - start) * t;
		}

		/*! \brief Linearly interpolates between two 3D vectors. \param start Start vector. \param end End vector. \param t Interpolation factor [0,1]. \return Interpolated vector. */
		template <typename T, typename U>
		constexpr Vector3D<T> lerp(const Vector3D<T>& start, const Vector3D<T>& end, U t)
		{
				return start + (end - start) * t;
		}

		/*! \brief Reflects a 2D incident vector off a surface with the given normal. \param incident The incoming vector. \param normal The surface normal (should be unit length). \return Reflected vector. */
		template <typename T>
		constexpr Vector2D<T> reflect(const Vector2D<T>& incident, const Vector2D<T>& normal)
		{
				return incident - 2 * dot(incident, normal) * normal;
		}

		/*! \brief Reflects a 3D incident vector off a surface with the given normal. \param incident The incoming vector. \param normal The surface normal (should be unit length). \return Reflected vector. */
		template <typename T>
		constexpr Vector3D<T> reflect(const Vector3D<T>& incident, const Vector3D<T>& normal)
		{
				return incident - 2 * dot(incident, normal) * normal;
		}

		/*! \brief Computes the angle between two 2D vectors in radians. \param lhs First vector. \param rhs Second vector. \return Angle in radians, or zero if either vector has zero magnitude. */
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

		/*! \brief Computes the angle between two 3D vectors in radians. \param lhs First vector. \param rhs Second vector. \return Angle in radians, or zero if either vector has zero magnitude. */
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

		/*! \brief Outputs a 2D vector to a stream in "(x, y)" format. \param os Output stream. \param vec The vector. \return Reference to the stream. */
		template <typename T>
		std::ostream& operator<<(std::ostream& os, const Vector2D<T>& vec)
		{
				os << "(" << vec.x << ", " << vec.y << ")";
				return os;
		}

		/*! \brief Outputs a 3D vector to a stream in "(x, y, z)" format. \param os Output stream. \param vec The vector. \return Reference to the stream. */
		template <typename T>
		std::ostream& operator<<(std::ostream& os, const Vector3D<T>& vec)
		{
				os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
				return os;
		}
}
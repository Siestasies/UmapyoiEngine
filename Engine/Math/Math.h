/*!
\file   Math.h
\par    Project: GAM200
\par    Course: CSD2401
\par    Section A
\par    Software Engineering Project 3

\author Jedrek Lee Jing Wei (100%)
\par    E-mail: jedrekjingwei.lee@digipen.edu
\par    DigiPen login: jedrekjingwei.lee

\brief
Convenience header wrapper providing type aliases and compile-time constants
for vector and matrix classes.

Includes Vector.h and Matrix.h, defining common
types (Vec2, Vec3, Mat2, Mat3, Mat4) with float, double, and int specializations,
along with unit vectors, zero vectors, identity matrices, and zero matrices.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once

#include "Vector.h"
#include "Matrix.h"

using namespace Uma_Math;

namespace Uma_Math
{
	using Vec2 = Vector2D<float>;
	using Vec3 = Vector3D<float>;
	
	using Vec2d = Vector2D<double>;
	using Vec3d = Vector3D<double>;
	
	using Vec2i = Vector2D<int>;
	using Vec3i = Vector3D<int>;

	using Mat2 = Matrix2x2<float>;
	using Mat3 = Matrix3x3<float>;
	using Mat4 = Matrix4x4<float>;

	using Mat2d = Matrix2x2<double>;
	using Mat3d = Matrix3x3<double>;
	using Mat4d = Matrix4x4<double>;
	
	using Mat2i = Matrix2x2<int>;
	using Mat3i = Matrix3x3<int>;
	using Mat4i = Matrix4x4<int>;
}

// Compile-time constants for different types, nice to have
namespace Uma_Constants
{
	// Vector
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

	// Matrix
	constexpr Matrix2x2<float> IDENTITY_2X2_F = Matrix2x2<float>::identity();
	constexpr Matrix2x2<float> ZERO_2X2_F = Matrix2x2<float>::zero();

	constexpr Matrix2x2<double> IDENTITY_2X2_D = Matrix2x2<double>::identity();
	constexpr Matrix2x2<double> ZERO_2X2_D = Matrix2x2<double>::zero();

	constexpr Matrix3x3<float> IDENTITY_3X3_F = Matrix3x3<float>::identity();
	constexpr Matrix3x3<float> ZERO_3X3_F = Matrix3x3<float>::zero();

	constexpr Matrix3x3<double> IDENTITY_3X3_D = Matrix3x3<double>::identity();
	constexpr Matrix3x3<double> ZERO_3X3_D = Matrix3x3<double>::zero();

	constexpr Matrix4x4<float> IDENTITY_4X4_F = Matrix4x4<float>::identity();
	constexpr Matrix4x4<float> ZERO_4X4_F = Matrix4x4<float>::zero();

	constexpr Matrix4x4<double> IDENTITY_4X4_D = Matrix4x4<double>::identity();
	constexpr Matrix4x4<double> ZERO_4X4_D = Matrix4x4<double>::zero();
}
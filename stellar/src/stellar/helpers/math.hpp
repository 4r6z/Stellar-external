#pragma once

#ifndef MATH_HPP
#define MATH_HPP

#include <cmath>
#include <vector>
#include "../helpers/tween.hpp"

static struct Vector4 final {
	float x, y, z, w;
};

static struct Vector3 final {
	float x, y, z;

	// Default constructor
	Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
	
	// Constructor with parameters
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

	float length() const {
		return sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2));
	}

	Vector3 normalize() const {
		float length = sqrt(pow(this->x, 2) + pow(this->y, 2) + pow(this->z, 2));
		
		if (length != 0)
			return { this->x / length, this->y / length, this->z / length };

		return { this->x, this->y, this->z };
	}

	Vector3 cross_product(const Vector3& b) const {
		return {
			this->y * b.z - this->z * b.y,
			this->z * b.x - this->x * b.z,
			this->x * b.y - this->y * b.x
		};
	}

	float dot_product(const Vector3& b) {
		return this->x * b.x + this->y * b.y + this->z * b.z;
	}

	Vector3 operator-(const Vector3& other) const {
		return { this->x - other.x, this->y - other.y, this->z - other.z };
	}

	Vector3 operator+(const Vector3& other) const {
		return { this->x + other.x, this->y + other.y, this->z + other.z };
	}

	Vector3 operator/(const Vector3& other) const {
		return { this->x / other.x, this->y / other.y, this->z / other.z };
	}

	Vector3 operator/(const float& other) const {
		return { this->x / other, this->y / other, this->z / other };
	}

	Vector3 operator*(const Vector3& other) const {
		return { this->x * other.x, this->y * other.y, this->z * other.z };
	}

	Vector3 operator*(const float& other) const {
		return { this->x * other, this->y * other, this->z * other };
	}

	bool operator==(const Vector3& other) const {
		return other.x == this->x && other.y == this->y && other.z == this->z;
	}

	bool operator==(const int& other) const {
		return this->x == other && this->y == other && this->z == other;
	}
};

static struct Vector2 final {
	float x, y;

	Vector2 operator-(const Vector2& other) const {
		return { this->x - other.x, this->y - other.y };
	}

	bool operator==(const int& other) const {
		return this->x == other && this->y == other;
	}
};

static struct Matrix4 final {
	float data[16];
};

static struct Matrix3 final { 
	float data[9]; 
};

static float calculate_distance(Vector2 a, Vector2 b) {
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

static float calculate_distance(Vector3 a, Vector3 b) {
	return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

static Matrix3 look_at_to_matrix(const Vector3& cam_pos, const Vector3& target_pos) {
	Vector3 forward = (target_pos - cam_pos).normalize();
	Vector3 right = Vector3{ 0, 1, 0 }.cross_product(forward).normalize();
	Vector3 up = forward.cross_product(right);

	Matrix3 matrix{};
	matrix.data[0] = (right.x * -1);  matrix.data[1] = up.x;  matrix.data[2] = -forward.x;
	matrix.data[3] = right.y;  matrix.data[4] = up.y;  matrix.data[5] = -forward.y;
	matrix.data[6] = (right.z * -1);  matrix.data[7] = up.z;  matrix.data[8] = -forward.z;

	return matrix;
}

static Matrix3 lerp_matrix(const Matrix3& a, const Matrix3& b, float t, float(*func)(float)) {
	Matrix3 result{};
	for (int i = 0; i < 9; ++i) {
		result.data[i] = a.data[i] + (b.data[i] - a.data[i]) * t;
	}
	return result;
}

static std::vector<Vector3> get_corners(const Vector3& partCF, const Vector3& partSize) {
	std::vector<Vector3> corners;

	for (int X = -1; X <= 1; X += 2) {
		for (int Y = -1; Y <= 1; Y += 2) {
			for (int Z = -1; Z <= 1; Z += 2) {
				Vector3 cornerPosition = {
					partCF.x + partSize.x * X,
					partCF.y + partSize.y * Y,
					partCF.z + partSize.z * Z
				};
				corners.push_back(cornerPosition);
			}
		}
	}

	return corners;
}

static bool is_point_in_polygon(const std::vector<Vector2>& polygon, const Vector2& point) {
	int n = polygon.size();
	bool inside = false;

	for (int i = 0, j = n - 1; i < n; j = i++) {
		if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
			(point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x)) {
			inside = !inside;
		}
	}

	return inside;
}

#endif
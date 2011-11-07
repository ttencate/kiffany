#ifndef MATHS_H
#define MATHS_H

#include "glm/glm.hpp"
#include "glm/ext.hpp"

using glm::dvec2;
using glm::dvec3;
using glm::dvec4;
using glm::int2;
using glm::int3;
using glm::int4;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::clamp;
using glm::cross;
using glm::dot;
using glm::exp;
using glm::floor;
using glm::fract;
using glm::inverse;
using glm::mix;
using glm::length;
using glm::normalize;
using glm::perspective;
using glm::round;
using glm::smoothstep;
using glm::translate;
using glm::transpose;
using glm::rotate;
using glm::value_ptr;

extern vec3 const X_AXIS;
extern vec3 const Y_AXIS;
extern vec3 const Z_AXIS;

extern int3 const X_STEP;
extern int3 const Y_STEP;
extern int3 const Z_STEP;

template<typename T>
T mod(T const &a, T const &b) {
	T r = a % b;
	return r < 0 ? r + b : r;
}

template<typename T>
T toRadians(T degrees) {
	return degrees / (2 * M_PI);
}

template<typename T>
inline T sqr(T x) {
	return x * x;
}

#endif

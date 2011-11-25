#ifndef MATHS_H
#define MATHS_H

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <iostream>

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
using glm::uvec2;
using glm::uvec3;
using glm::uvec4;

using glm::clamp;
using glm::cross;
using glm::dot;
using glm::exp;
using glm::floor;
using glm::fract;
using glm::inverse;
using glm::log;
using glm::mix;
using glm::mod;
using glm::length;
using glm::normalize;
using glm::perspective;
using glm::pow;
using glm::pow2;
using glm::radians;
using glm::round;
using glm::sign;
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
inline T sqr(T x) {
	return x * x;
}

template<typename T>
std::ostream &operator<<(std::ostream &stream, glm::detail::tvec2<T> const &v) {
	stream << glm::to_string(v);
	return stream;
}

template<typename T>
std::ostream &operator<<(std::ostream &stream, glm::detail::tvec3<T> const &v) {
	stream << glm::to_string(v);
	return stream;
}

template<typename T>
std::ostream &operator<<(std::ostream &stream, glm::detail::tvec4<T> const &v) {
	stream << glm::to_string(v);
	return stream;
}

#endif

#ifndef VECTOR_H
#define VECTOR_H

#include "functions.h"

#include <cstring>
#include <cmath>
#include <iostream>

template<typename C, typename T, size_t N>
class VectorBase {
	
	T c[N];
	
	public:
		
		VectorBase();
		
		T &operator[](size_t index) { return c[index]; }
		T const &operator[](size_t index) const { return c[index]; }
		
		T *data() { return c; }
		T const *data() const { return c; }
		
		void normalize();
		C normalized() const;
		T norm() const;
		T norm2() const;

		bool isZero() const;
};

template<typename C, typename T, size_t N>
VectorBase<C, T, N>::VectorBase() {
	memset(c, 0, N * sizeof(T));
}

template<typename C, typename T, size_t N>
void VectorBase<C, T, N>::normalize() {
	T n = norm();
	for (size_t i = 0; i < N; ++i)
		c[i] /= n;
}

template<typename C, typename T, size_t N>
C VectorBase<C, T, N>::normalized() const {
	C n(*static_cast<C const*>(this));
	n.normalize();
	return n;
}

template<typename C, typename T, size_t N>
T VectorBase<C, T, N>::norm2() const {
	T n = 0;
	for (size_t i = 0; i < N; ++i)
		n += sqr(c[i]);
	return n;
}

template<typename C, typename T, size_t N>
T VectorBase<C, T, N>::norm() const {
	return sqrtt(norm2());
}

template<typename C, typename T, size_t N>
bool VectorBase<C, T, N>::isZero() const {
	for (size_t i = 0; i < N; ++i)
		if (c[i] != 0)
			return false;
	return true;
}

// --------------------------------------------------------------------------

template<typename T, size_t N>
class Vector : public VectorBase<Vector<T, N>, T, N> {
};

template<typename T, size_t N>
Vector<T, N> &operator+=(Vector<T, N> &l, Vector<T, N> const &r) {
	for (size_t i = 0; i < N; ++i)
		l[i] += r[i];
	return l;
}

template<typename T, size_t N>
Vector<T, N> &operator-=(Vector<T, N> &l, Vector<T, N> const &r) {
	for (size_t i = 0; i < N; ++i)
		l[i] -= r[i];
	return l;
}

template<typename T, size_t N>
Vector<T, N> &operator*=(Vector<T, N> &l, Vector<T, N> const &r) {
	for (size_t i = 0; i < N; ++i)
		l[i] *= r[i];
	return l;
}

template<typename T, size_t N>
Vector<T, N> &operator/=(Vector<T, N> &l, Vector<T, N> const &r) {
	for (size_t i = 0; i < N; ++i)
		l[i] /= r[i];
	return l;
}

template<typename T, size_t N>
Vector<T, N> &operator*=(Vector<T, N> &l, T const &r) {
	for (size_t i = 0; i < N; ++i)
		l[i] *= r;
	return l;
}

template<typename T, size_t N>
Vector<T, N> &operator/=(Vector<T, N> &l, T const &r) {
	for (size_t i = 0; i < N; ++i)
		l[i] /= r;
	return l;
}

template<typename T, size_t N>
Vector<T, N> operator+(Vector<T, N> a, Vector<T, N> const &b) {
	a += b;
	return a;
}

template<typename T, size_t N>
Vector<T, N> operator-(Vector<T, N> a, Vector<T, N> const &b) {
	a -= b;
	return a;
}

template<typename T, size_t N>
Vector<T, N> operator*(Vector<T, N> a, Vector<T, N> const &b) {
	a *= b;
	return a;
}

template<typename T, size_t N>
Vector<T, N> operator/(Vector<T, N> a, Vector<T, N> const &b) {
	a /= b;
	return a;
}

template<typename T, size_t N>
Vector<T, N> operator*(T const &t, Vector<T, N> v) {
	v *= t;
	return v;
}

template<typename T, size_t N>
Vector<T, N> operator*(Vector<T, N> v, T const &t) {
	v *= t;
	return v;
}

template<typename T, size_t N>
Vector<T, N> operator/(Vector<T, N> v, T const &t) {
	v /= t;
	return v;
}

template<typename T, size_t N>
Vector<T, N> operator+(Vector<T, N> const &v) {
	return v;
}

template<typename T, size_t N>
Vector<T, N> operator-(Vector<T, N> const &v) {
	Vector<T, N> n;
	for (size_t i = 0; i < N; ++i)
		n[i] = -v[i];
	return n;
}

template<typename T, size_t N>
bool operator!=(Vector<T, N> const &l, Vector<T, N> const &r) {
	for (size_t i = 0; i < N; ++i)
		if (l[i] == r[i])
			return false;
	return true;
}

template<typename T, size_t N>
bool operator==(Vector<T, N> const &l, Vector<T, N> const &r) {
	for (size_t i = 0; i < N; ++i)
		if (l[i] != r[i])
			return false;
	return true;
}

template<typename T, size_t N>
T dot(Vector<T, N> const &l, Vector<T, N> const &r) {
	T d = 0;
	for (size_t i = 0; i < N; ++i)
		d += l[i] * r[i];
	return d;
}

template<typename T, size_t N>
T distance2(Vector<T, N> const &a, Vector<T, N> const &b) {
	return (b - a).norm2();
}

template<typename T, size_t N>
T distance(Vector<T, N> const &a, Vector<T, N> const &b) {
	return (b - a).norm();
}

// --------------------------------------------------------------------------

template<typename T>
class Vector<T, 2> : public VectorBase<Vector<T, 2>, T, 2> {
	public:
		Vector() { }
		Vector(T x_, T y_) { x(x_); y(y_); }
		T x() const { return (*this)[0]; }
		T y() const { return (*this)[1]; }
		void x(T v) { (*this)[0] = v; }
		void y(T v) { (*this)[1] = v; }
};

template<typename T>
class Vector<T, 3> : public VectorBase<Vector<T, 3>, T, 3> {
	public:
		Vector() { }
		Vector(T x_, T y_, T z_) { x(x_); y(y_); z(z_); }
		T x() const { return (*this)[0]; }
		T y() const { return (*this)[1]; }
		T z() const { return (*this)[2]; }
		void x(T v) { (*this)[0] = v; }
		void y(T v) { (*this)[1] = v; }
		void z(T v) { (*this)[2] = v; }
};

template<typename T>
class Vector<T, 4> : public VectorBase<Vector<T, 4>, T, 4> {
	public:
		Vector() { }
		Vector(T x_, T y_, T z_, T w_) { x(x_); y(y_); z(z_); w(w_); }
		T x() const { return (*this)[0]; }
		T y() const { return (*this)[1]; }
		T z() const { return (*this)[2]; }
		T w() const { return (*this)[3]; }
		void x(T v) { (*this)[0] = v; }
		void y(T v) { (*this)[1] = v; }
		void z(T v) { (*this)[2] = v; }
		void w(T v) { (*this)[3] = v; }
};

// --------------------------------------------------------------------------

typedef Vector<float, 2> float2;
typedef Vector<float, 3> float3;
typedef Vector<float, 4> float4;
typedef Vector<int, 2> int2;
typedef Vector<int, 3> int3;

#endif

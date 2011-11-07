#ifndef TABLE_H
#define TABLE_H

#include "maths.h"

#include <boost/scoped_array.hpp>

template<typename T>
class Table {

	protected:

		unsigned const size;
		boost::scoped_array<T> const values;

	public:

		Table(unsigned size)
		:
			size(size),
			values(new T[size])
		{
		}

		Table(Table const &other)
		:
			size(other.size),
			values(new T[size])
		{
			for (unsigned i = 0; i < size; ++i) {
				values[i] = other.values[i];
			}
		}

		unsigned getSize() const { return size; }

		T &operator[](unsigned index) { return values[index]; }
		T const &operator[](unsigned index) const { return values[index]; }

		T *raw() { return values.get(); }
		T const *raw() const { return values.get(); }

};

template<typename T>
class Table2D
:
	public Table<T>
{

	unsigned const nx;
	unsigned const ny;

	public:

		typedef vec2 Coords;

		Table2D(unsigned nx, unsigned ny)
		:
			Table<T>(nx * ny),
			nx(nx),
			ny(ny)
		{
		}

		T operator()(vec2 pos) const;

};

template<typename T>
class Table3D
:
	public Table<T>
{

	unsigned const nx;
	unsigned const ny;
	unsigned const nz;

	public:

		typedef vec3 Coords;

		Table3D(unsigned nx, unsigned ny, unsigned nz)
		:
			Table<T>(nx * ny * nz),
			nx(nx),
			ny(ny),
			nz(nz)
		{
		}

		T operator()(vec3 pos) const;

};

template<typename T>
T Table2D<T>::operator()(vec2 pos) const {
	int const FY = nx;
	vec2 p = mod(pos - 0.5f, vec2(nx, ny));
	int xa = (int)p.x;
	int xb = (xa + 1) % nx;
	int ya = (int)p.y;
	int yb = (ya + 1) % ny;
	vec2 f = fract(p);
	T const *values = this->raw();
	return mix(
			mix(values[xa + FY * ya], values[xb + FY * ya], f.x),
			mix(values[xa + FY * yb], values[xb + FY * yb], f.x),
			f.y);
}

template<typename T>
T Table3D<T>::operator()(vec3 pos) const {
	int const FY = nx;
	int const FZ = nx * ny;
	vec3 p = mod(pos - 0.5f, vec3(nx, ny, nz));
	int xa = (int)p.x;
	int xb = (xa + 1) % nx;
	int ya = (int)p.y;
	int yb = (ya + 1) % ny;
	int za = (int)p.z;
	int zb = (za + 1) % nz;
	vec3 f = fract(p);
	T const *values = this->raw();
	return mix(
			mix(
				mix(values[xa + FY * ya + FZ * za], values[xb + FY * ya + FZ * za], f.x),
				mix(values[xa + FY * yb + FZ * za], values[xb + FY * yb + FZ * za], f.x),
				f.y),
			mix(
				mix(values[xa + FY * ya + FZ * zb], values[xb + FY * ya + FZ * zb], f.x),
				mix(values[xa + FY * yb + FZ * zb], values[xb + FY * yb + FZ * zb], f.x),
				f.y),
			f.z);
}

#endif

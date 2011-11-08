#ifndef TABLE_H
#define TABLE_H

#include "maths.h"

#include <boost/scoped_array.hpp>

template<typename T>
class Array {

	unsigned const numCells;
	boost::scoped_array<T> const values;

	public:

		typedef T value_type;

		Array(unsigned numCells)
		:
			numCells(numCells),
			values(new T[numCells])
		{
		}

		Array(Array<T> const &other)
		:
			numCells(other.numCells),
			values(new T[numCells])
		{
			for (unsigned i = 0; i < numCells; ++i) {
				values[i] = other.values[i];
			}
		}

		unsigned getNumCells() const { return numCells; }

		T &operator[](unsigned index) { return values[index]; }
		T const &operator[](unsigned index) const { return values[index]; }

		T *begin() { return &values[0]; }
		T *end() { return &values[numCells]; }
		T const *begin() const { return &values[0]; }
		T const *end() const { return &values[numCells]; }

		T *raw() { return values.get(); }
		T const *raw() const { return values.get(); }

};

template<typename T, typename C>
inline T lerp(C pos, uvec2 size, T const *values) {
	int const FY = size[0];
	C p = mod(pos, C(size));
	int xa = (int)p.x;
	int xb = (xa + 1) % size[0];
	int ya = (int)p.y;
	int yb = (ya + 1) % size[1];
	C f = fract(p);
	return mix(
			mix(values[xa + FY * ya], values[xb + FY * ya], f.x),
			mix(values[xa + FY * yb], values[xb + FY * yb], f.x),
			f.y);
}

template<typename T, typename C>
inline T lerp(C pos, uvec3 size, T const *values) {
	int const FY = size[0];
	int const FZ = size[0] * size[1];
	C p = mod(pos, C(size));
	int xa = (int)p.x;
	int xb = (xa + 1) % size[0];
	int ya = (int)p.y;
	int yb = (ya + 1) % size[1];
	int za = (int)p.z;
	int zb = (za + 1) % size[2];
	C f = fract(p);
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

template<typename T, typename S, typename C>
class Table
:
	public Array<T>
{

	public:

		typedef S size_type;
		typedef C coords_type;

	private:

		size_type const size;
		coords_type const scale;
		coords_type const offset;

		static unsigned computeNumCells(uvec2 size) { return size.x * size.y; }
		static unsigned computeNumCells(uvec3 size) { return size.x * size.y * size.z; }

	public:

		Table(size_type size, coords_type scale = coords_type(1), coords_type offset = coords_type(0.5))
		:
			Array<T>(computeNumCells(size)),
			size(size),
			scale(scale),
			offset(offset)
		{
		}

		Table(Table<T, S, C> const &other)
		:
			Array<T>(other),
			size(other.size),
			scale(other.scale),
			offset(other.offset)
		{
		}

		static Table<T, S, C> createWithCoordsSize(size_type size, coords_type coordsSize) {
			return Table<T, S, C>(size, size / coordsSize);
		}

		static Table<T, S, C> createWithCoordsSizeAndOffset(size_type size, coords_type coordsSize, coords_type offset) {
			return Table<T, S, C>(size, size / coordsSize, offset);
		}

		T operator()(coords_type pos) const {
			return lerp(pos * scale - offset, size, this->raw());
		}

};

template<typename T>
struct Table2D {
	typedef Table<T, uvec2, vec2> type;
};

template<typename T>
struct Table3D {
	typedef Table<T, uvec3, vec3> type;
};

typedef Table2D<float>::type FloatTable2D;
typedef Table3D<float>::type FloatTable3D;

#endif

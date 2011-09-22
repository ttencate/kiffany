#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <iomanip>
#include <cstring>

#include "functions.hpp"

template<typename T>
class Matrix {
	
	size_t d_nx, d_ny;
	T *d_cells;
	
	public:
		
		Matrix(size_t nx, size_t ny);
		Matrix(Matrix<T> const &other);
		~Matrix();
		
		size_t nx() const { return d_nx; }
		size_t ny() const { return d_ny; }
		T &operator()(size_t x, size_t y) { SLOW_ASSERT(x < d_nx); SLOW_ASSERT(y < d_ny); return d_cells[d_nx * y + x]; }
		T const &operator()(size_t x, size_t y) const { SLOW_ASSERT(x < d_nx); SLOW_ASSERT(y < d_ny); return d_cells[d_nx * y + x]; }
		
		void setAll(T const &value) { setRect(0, 0, d_nx, d_ny, value); }
		void setInternal(T const &value) { setRect(1, 1, d_nx - 2, d_ny - 2, value); }
		void setBottom(T const &value) { setRect(0, 0, d_nx, 1, value); }
		void setTop(T const &value) { setRect(0, d_ny - 1, d_nx, 1, value); }
		void setLeft(T const &value) { setRect(0, 0, 1, d_ny, value); }
		void setRight(T const &value) { setRect(d_nx - 1, 0, 1, d_ny, value); }
		void setBorders(T const &value) { setBottom(value); setTop(value); setLeft(value); setRight(value); }
		void setRect(size_t tx, size_t ty, size_t nx, size_t ny, T const &value) {
			for (size_t x = 0; x < nx; ++x) {
				for (size_t y = 0; y < ny; ++y) {
					(*this)(tx + x, ty + y) = value;
				}
			}
		}
		void copyRect(Matrix<T> const &from, size_t fx, size_t fy, size_t tx, size_t ty, size_t nx, size_t ny) {
			for (size_t x = 0; x < nx; ++x) {
				for (size_t y = 0; y < ny; ++y) {
					(*this)(tx + x, ty + y) = from(fx + x, fy + y);
				}
			}
		}
		void negateCopyRect(Matrix<T> const &from, size_t fx, size_t fy, size_t tx, size_t ty, size_t nx, size_t ny) {
			for (size_t x = 0; x < nx; ++x) {
				for (size_t y = 0; y < ny; ++y) {
					(*this)(tx + x, ty + y) = -from(fx + x, fy + y);
				}
			}
		}
		
		T const *data() const { return d_cells; }
		T *writableData() { return d_cells; }
		inline T &operator[](size_t i) { return d_cells[i]; }
		inline T const &operator[](size_t i) const { return d_cells[i]; }
:qa
		
		Matrix<T> &operator=(Matrix<T> const &other);
		void swap(Matrix<T> &other);
		
	private:
	
};

template<typename T>
Matrix<T>::Matrix(size_t nx, size_t ny) :
	d_nx(nx),
	d_ny(ny),
	d_cells(new T[d_nx * d_ny])
{
}

template<typename T>
Matrix<T>::Matrix(Matrix const &other) :
	d_nx(other.d_nx),
	d_ny(other.d_ny),
	d_cells(new T[d_nx * d_ny])
{
	memcpy(d_cells, other.d_cells, sizeof(T) * d_nx * d_ny);
}

template<typename T>
Matrix<T>::~Matrix() {
	delete[] d_cells;
}

template<typename T>
T Matrix<T>::bilerp(float x, float y) const {
	int w = (size_t)(x - 0.5f);
	int e = (size_t)(x + 0.5f);
	int s = (size_t)(y - 0.5f);
	int n = (size_t)(y + 0.5f);
	clamp<int>(0, w, d_nx - 1);
	clamp<int>(0, e, d_nx - 1);
	clamp<int>(0, s, d_ny - 1);
	clamp<int>(0, n, d_ny - 1);
	float ce = x - w;
	float cw = 1.0f - ce;
	float cn = y - s;
	float cs = 1.0f - cn;
	return
		cw * cs * (*this)(w, s) +
		ce * cs * (*this)(e, s) +
		cw * cn * (*this)(w, n) + 
		ce * cn * (*this)(e, n);
}

template<typename T>
Matrix<T> &Matrix<T>::operator=(Matrix<T> const &other) {
	if (d_nx == other.d_nx && d_ny == other.d_ny) {
		memcpy(d_cells, other.d_cells, sizeof(T) * d_nx * d_ny);
	} else {
		Matrix<T> temp(other);
		swap(temp);
	}
	return *this;
}

template<typename T>
void Matrix<T>::swap(Matrix<T> &other) {
	char temp[sizeof(Matrix<T>)];
	memcpy(temp, &other, sizeof(Matrix<T>));
	memcpy(&other, this, sizeof(Matrix<T>));
	memcpy(this, temp, sizeof(Matrix<T>));
}

template<typename T>
std::ostream &operator<<(std::ostream &out, Matrix<T> const &matrix) {
	for (int y = matrix.ny() - 1; y >= 0; --y) {
		for (size_t x = 0; x < matrix.nx(); ++x) {
			out << std::setw(6) << matrix(x, y) << ' ';
		}
		out << '\n';
	}
	return out;
}

#endif

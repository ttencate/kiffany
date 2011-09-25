#ifndef COORDS_H
#define COORDS_H

#include "maths.h"

class CoordsBlock {

	int3 const size;
	int3 const offset;

	class Iterator {

		CoordsBlock const *const parent;

		int3 relCoords;

		public:

			Iterator(CoordsBlock const *parent, int3 relCoords);

			Iterator &operator++();
			Iterator operator++(int);

			Iterator &operator+=(int delta);
			Iterator operator+(int delta) const;

			bool operator==(Iterator const &other) const;
			bool operator!=(Iterator const &other) const;

			int3 operator*() const;
		
		private:

			void wrap();

	};

	public:

		typedef Iterator const_iterator;
	
		CoordsBlock(int3 const &size, int3 const &offset = int3(0));

		const_iterator begin() const;
		const_iterator end() const;

};

inline vec3 blockMin(int3 const &pos) {
	return vec3(pos);
}

inline vec3 blockMax(int3 const &pos) {
	return vec3(pos) + vec3(1.0f);
}

inline vec3 blockCenter(int3 const &pos) {
	return vec3(pos.x + 0.5f, pos.y + 0.5f, pos.z + 0.5f);
}

#endif

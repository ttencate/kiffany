#ifndef COORDS_H
#define COORDS_H

#include "maths.h"

extern const unsigned CHUNK_SIZE;

extern const unsigned BLOCKS_PER_CHUNK;

extern const float CHUNK_RADIUS;

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

inline int3 chunkIndexFromPosition(vec3 const &position) {
	return int3(floor(position / (float)CHUNK_SIZE));
}

inline vec3 chunkMin(int3 const &index) {
	return (float)CHUNK_SIZE * vec3(index);
}

inline vec3 chunkMax(int3 const &index) {
	return (float)CHUNK_SIZE * vec3(index + 1);
}

inline vec3 chunkCenter(int3 const &index) {
	return (float)CHUNK_SIZE * (vec3(index) + 0.5f);
}

#endif

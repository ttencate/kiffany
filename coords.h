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

vec3 blockMin(int3 const &pos);
vec3 blockMax(int3 const &pos);
vec3 blockCenter(int3 const &pos);

#endif

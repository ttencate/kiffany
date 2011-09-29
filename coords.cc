#include "coords.h"

const unsigned CHUNK_SIZE = 32;

const unsigned BLOCKS_PER_CHUNK = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

CoordsBlock::Iterator::Iterator(CoordsBlock const *parent, int3 relCoords)
:
	parent(parent),
	relCoords(relCoords)
{
}

CoordsBlock::Iterator &CoordsBlock::Iterator::operator++() {
	++relCoords.x;
	wrap();
	return *this;
}

CoordsBlock::Iterator CoordsBlock::Iterator::operator++(int) {
	CoordsBlock::Iterator old = *this;
	++*this;
	return old;
}

CoordsBlock::Iterator &CoordsBlock::Iterator::operator+=(int delta) {
	relCoords.x += delta;
	wrap();
	return *this;
}

CoordsBlock::Iterator CoordsBlock::Iterator::operator+(int delta) const {
	return CoordsBlock::Iterator(*this) += delta;
}

bool CoordsBlock::Iterator::operator==(CoordsBlock::Iterator const &other) const {
	return relCoords == other.relCoords;
}

bool CoordsBlock::Iterator::operator!=(CoordsBlock::Iterator const &other) const {
	return !(*this == other);
}

int3 CoordsBlock::Iterator::operator*() const {
	return parent->offset + relCoords;
}

void CoordsBlock::Iterator::wrap() {
	relCoords.y += relCoords.x / parent->size.x;
	relCoords.x %= parent->size.x;
	relCoords.z += relCoords.y / parent->size.y;
	relCoords.y %= parent->size.y;
}

CoordsBlock::CoordsBlock(int3 const &size, int3 const &offset)
:
	size(size),
	offset(offset)
{
}

CoordsBlock::const_iterator CoordsBlock::begin() const {
	return Iterator(this, int3(0));
}

CoordsBlock::const_iterator CoordsBlock::end() const {
	return Iterator(this, int3(0, 0, size.z));
}


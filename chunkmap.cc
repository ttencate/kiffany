#include "chunkmap.h"

#include "stats.h"

ChunkMap::ChunkMap() {
}

ChunkPtr ChunkMap::operator[](int3 index) {
	boost::upgrade_lock<boost::shared_mutex> readLock(mapMutex);
	PositionMap::iterator i = map.find(index);
	if (i == map.end()) {
		ChunkPtr chunk(new Chunk(index));
		stats.chunksCreated.increment();

		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);

			map[index] = chunk;
		}

		return chunk;
	}
	return i->second;
}

ChunkConstPtr ChunkMap::operator[](int3 index) const {
	boost::shared_lock<boost::shared_mutex> readLock(mapMutex);
	PositionMap::const_iterator i = map.find(index);
	if (i == map.end()) {
		return ChunkConstPtr();
	}
	return i->second;
}

bool ChunkMap::contains(int3 index) const {
	boost::shared_lock<boost::shared_mutex> readLock(mapMutex);
	return map.find(index) != map.end();
}

Chunk::State ChunkMap::getChunkState(int3 index) const {
	boost::shared_lock<boost::shared_mutex> readLock(mapMutex);
	PositionMap::const_iterator i = map.find(index);
	if (i == map.end()) {
		return Chunk::NEW;
	}
	return i->second->getState();
}

bool ChunkMap::isChunkUpgrading(int3 index) const {
	boost::shared_lock<boost::shared_mutex> readLock(mapMutex);
	PositionMap::const_iterator i = map.find(index);
	if (i == map.end()) {
		return false;
	}
	return i->second->isUpgrading();
}

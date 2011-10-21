#include "chunkmap.h"

#include "stats.h"

float ChunkPriorityFunction::operator()(Chunk const &chunk) const {
	return (*this)(chunk.getIndex());
}

float ChunkPriorityFunction::operator()(int3 index) const {
	return (*this)(chunkCenter(index));
}

float ChunkPriorityFunction::operator()(vec3 chunkCenter) const {
	return 1 / length(chunkCenter - camera.getPosition());
}

ChunkMap::ChunkMap(unsigned maxSize)
:
	maxSize(maxSize)
{
}

ChunkPtr ChunkMap::operator[](int3 index) {
	boost::upgrade_lock<boost::shared_mutex> readLock(mapMutex);
	PositionMap::iterator i = map.find(index);
	if (i == map.end()) {
		float priority = evictionQueue.priority(index);
		if (atCapacity() && (evictionQueue.empty() || priority <= evictionQueue.back_priority())) {
			return ChunkPtr();
		}

		ChunkPtr chunk(new Chunk(index));
		stats.chunksCreated.increment();

		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> writeLock(readLock);

			map[index] = chunk;
			evictionQueue.insert(index);
			trim();
		}

		return chunk;
	}
	return i->second;
}

bool ChunkMap::contains(int3 index) const {
	boost::shared_lock<boost::shared_mutex> readLock(mapMutex);
	return map.find(index) != map.end();
}

void ChunkMap::setPriorityFunction(ChunkPriorityFunction const &priorityFunction) {
	evictionQueue.setPriorityFunction(priorityFunction);
}

void ChunkMap::trim() {
	// Caution: assumes that we hold the write lock!
	while (overCapacity()) {
		stats.chunksEvicted.increment();
		int3 index = evictionQueue.back();
		evictionQueue.pop_back();
		map.erase(index);
	}
}

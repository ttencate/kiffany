#ifndef TERRAIN_H
#define TERRAIN_H

#include "camera.h"
#include "chunk.h"
#include "terragen.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include <list>
#include <utility>

#include <iostream>

struct CoordsHasher {
	size_t operator()(int3 const &p) const;
};

template<typename K, typename V, typename Hash = boost::hash<K> >
class LruMap {

	typedef std::pair<K, V> Pair;
	typedef std::list<Pair> List;
	typedef boost::unordered_map<K, typename List::iterator, Hash> Map;

	List list;
	Map map;

	public:

		V &operator[](K const &key) {
			typename Map::iterator i = map.find(key);
			Pair p;
			if (i == map.end()) {
				p = std::make_pair(key, V());
			} else {
				p = *i->second;
				list.erase(i->second);
			}
			list.push_front(p);
			map[key] = list.begin();
			return list.front().second;
		}

		void pop_back() {
			map.erase(list.back().first);
			list.pop_back();
		}

		size_t size() const {
			return map.size();
		}

};

class ChunkMap
:
	boost::noncopyable
{

	unsigned const maxSize;

	typedef LruMap<int3, ChunkPtr, CoordsHasher> Map;

	Map map;

	public:

		ChunkMap(unsigned maxSize);

		ChunkPtr get(int3 const &index);

	private:

		void trimToSize();

};

class Terrain
:
	boost::noncopyable
{

	boost::scoped_ptr<TerrainGenerator> terrainGenerator;
	AsyncTerrainGenerator asyncTerrainGenerator;

	ChunkMap chunkMap;

	public:

		Terrain(TerrainGenerator *terrainGenerator);
		~Terrain();

		void update(float dt);
		void render(Camera const &camera);

	private:

		unsigned computeMaxNumChunks() const;
		void renderChunk(Camera const &camera, int3 const &index);

};

#endif

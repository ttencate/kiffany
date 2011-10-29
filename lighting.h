#ifndef LIGHTING_H
#define LIGHTING_H

#include "chunk.h"
#include "chunkmap.h"
#include "maths.h"

void computeLighting(int3 index, ChunkMap const &chunkMap, ChunkGeometryPtr geometry);

#endif

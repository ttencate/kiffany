#include "terrain.h"

#include "atmosphere.h"
#include "flags.h"
#include "lighting.h"
#include "space.h"
#include "stats.h"
#include "terragen.h"

#include <boost/assert.hpp>

Terrain::Terrain(TerrainGenerator *terrainGenerator)
:
	chunkMap(),
	chunkManager(chunkMap, terrainGenerator)
{
	shaderProgram.loadAndLink("shaders/terrain.vert", "shaders/terrain.frag");
}

Terrain::~Terrain() {
}

void Terrain::addViewSphere(WeakConstViewSpherePtr viewSphere) {
	chunkManager.addViewSphere(viewSphere);
}

void Terrain::update(float dt) {
	chunkManager.sow();
	chunkManager.reap();
}

void Terrain::render(Camera const &camera, Lighting const &lighting) {
	GLAtmosphere const &atmosphere = lighting.getAtmosphere();
	AtmosParams const &params = atmosphere.getParams();
	Sun const &sun = lighting.getSun();

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	useProgram(shaderProgram.getProgram());
	shaderProgram.setUniform("material.ambient", vec4(0.5f, 0.5f, 0.5f, 1.0f));
	shaderProgram.setUniform("material.diffuse", vec4(0.5f, 0.5f, 0.5f, 1.0f));
	shaderProgram.setUniform("lighting.ambientColor", lighting.ambientColor());
	shaderProgram.setUniform("sun.color", vec3(sun.getColor()));
	shaderProgram.setUniform("sun.direction", sun.getDirection());
	shaderProgram.setUniform("params.rayleighCoefficient", params.rayleighCoefficient);
	shaderProgram.setUniform("params.mieCoefficient", params.mieCoefficient);
	shaderProgram.setUniform("params.mieDirectionality", params.mieDirectionality);
	shaderProgram.setUniform("params.numAngles", (int)params.numAngles);
	shaderProgram.setUniform("cameraPosition", camera.getPosition());
	bindFragDataLocation(shaderProgram.getProgram(), 0, "color");

	activeTexture(0);
	bindTexture(GL_TEXTURE_RECTANGLE, atmosphere.getTotalTransmittanceTexture());
	shaderProgram.setUniform("totalTransmittanceSampler", 0);

	int3 center = chunkIndexFromPoint(camera.getPosition());
	int radius = flags.viewDistance / CHUNK_SIZE;
	for (int z = -radius; z <= radius; ++z) {
		for (int y = -radius; y <= radius; ++y) {
			for (int x = -radius; x <= radius; ++x) {
				// TODO sphere check
				int3 const index = center + int3(x, y, z);
				// TODO do this properly by emulating the matrix stack
				shaderProgram.setUniform("vertexOffset", chunkMin(index));
				renderChunk(camera, index);
			}
		}
	}
}

// TODO this is currently unused
unsigned Terrain::computeMaxNumChunks() const {
	if (flags.maxNumChunks != 0) {
		return flags.maxNumChunks;
	}
	unsigned radius = flags.viewDistance / CHUNK_SIZE;
	unsigned size = 2 * radius + 1;
	return size * size * size;
}

void Terrain::renderChunk(Camera const &camera, int3 const &index) {
	// TODO avoid creating 'em (change [] semantics?)
	ChunkPtr chunk = chunkMap[index];
	if (!chunk) {
		return;
	}
	stats.chunksConsidered.increment();
	if (chunk->getState() < Chunk::TESSELATED) {
		stats.chunksSkipped.increment();
	} else {
		if (camera.isSphereInView(chunkCenter(index), CHUNK_RADIUS)) {
			chunk->render();
		} else {
			stats.chunksCulled.increment();
		}
	}
}

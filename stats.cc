#include "stats.h"

#include <iostream>

Stats stats;

void Stats::print() const {
	std::cout
		<< "Chunks created: " << chunksCreated.get() << '\n'
		<< "Chunks generated: " << chunksGenerated.get() << '\n'
		<< "Time per chunk: " << (chunkGenerationTime.get() / chunksGenerated.get()) << '\n'
		<< "Tesselations done: " << chunksTesselated.get() << '\n'
		<< "Time per chunk: " << (chunkTesselationTime.get() / chunksTesselated.get()) << '\n'
		<< "Chunks evicted: " << chunksEvicted.get() << '\n'
		<< "Irrelevant jobs skipped: " << irrelevantJobsSkipped.get() << '\n'
		<< "Irrelevant jobs run: " << irrelevantJobsRun.get() << '\n'
		<< '\n'
		<< "Quads generated: " << quadsGenerated.get() << '\n'
		<< "Quads per chunk: " << ((float)quadsGenerated.get() / chunksGenerated.get()) << '\n'
		<< "Buffers created: " << buffersCreated.get() << '\n'
		<< "Buffers deleted: " << buffersDeleted.get() << '\n'
		<< '\n'
		<< "Running time: " << runningTime.get() << '\n'
		<< "Frames rendered: " << framesRendered.get() << '\n'
		<< "Frames per second: " << (framesRendered.get() / runningTime.get()) << '\n'
		<< '\n'
		<< "Chunks considered for rendering: " << chunksConsidered.get() << '\n'
		<< "Chunks skipped: " << chunksSkipped.get() << '\n'
		<< "Chunks culled: " << chunksCulled.get() << '\n'
		<< "Chunks empty: " << chunksEmpty.get() << '\n'
		<< "Chunks rendered: " << chunksRendered.get() << '\n'
		<< "Skipped fraction: " << ((float)chunksSkipped.get() / chunksConsidered.get()) << '\n'
		<< "Culled fraction: " << ((float)chunksCulled.get() / chunksConsidered.get()) << '\n'
		<< "Empty fraction: " << ((float)chunksEmpty.get() / chunksConsidered.get()) << '\n'
		<< "Rendered fraction: " << ((float)chunksRendered.get() / chunksConsidered.get()) << '\n'
		<< '\n'
		<< "Chunks per frame: " << ((float)chunksRendered.get() / framesRendered.get()) << '\n'
		<< "Chunks per second: " << (chunksRendered.get() / runningTime.get()) << '\n'
		<< "Quads rendered: " << quadsRendered.get() << '\n'
		<< "Quads per frame: " << ((float)quadsRendered.get() / framesRendered.get()) << '\n'
		<< "Quads per second: " << (quadsRendered.get() / runningTime.get()) << '\n'
		;
}

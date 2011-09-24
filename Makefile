.PHONY all: kiffany

kiffany: block.cc camera.cc chunk.cc gl.cc main.cc maths.cc terrain.cc terragen.cc world.cc
	g++ -g -o$@ -lglfw -lGLEW -lGL $^

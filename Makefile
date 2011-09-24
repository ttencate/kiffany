.PHONY all: kiffany

kiffany: camera.cc chunk.cc gl.cc main.cc maths.cc terrain.cc world.cc
	g++ -g -o$@ -lglfw -lGLEW -lGL $^

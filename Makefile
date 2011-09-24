.PHONY all: kiffany

kiffany: main.cc maths.cc camera.cc chunk.cc terrain.cc world.cc buffer.cc
	g++ -g -o$@ -lglfw -lGLEW -lGL $^

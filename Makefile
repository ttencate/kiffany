.PHONY all: kiffany

kiffany: main.cc chunk.cc world.cc
	g++ -g -o$@ -lglfw -lGLEW -lGL $^

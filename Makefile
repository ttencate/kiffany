.PHONY all: kiffany

kiffany: main.cc chunk.cc world.cc buffer.cc
	g++ -g -o$@ -lglfw -lGLEW -lGL $^

.PHONY all: kiffany

kiffany: main.cc functions.cc chunk.cc world.cc
	g++ -o$@ -lglfw -lGL $^

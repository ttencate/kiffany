.PHONY all: kiffany

kiffany: main.cc
	g++ -o$@ -lglfw -lGL $^

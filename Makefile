CXX=colorgcc

CC_FILES=\
  block.cc \
  camera.cc \
  chunk.cc \
  chunkdata.cc \
  coords.cc \
  gl.cc \
  main.cc \
  maths.cc \
  stats.cc \
  terrain.cc \
  terragen.cc \
  world.cc

H_FILES=$(CC_FILES:.cc=.h)

.PHONY all: kiffany

kiffany: $(CC_FILES) $(H_FILES)
	$(CXX) -g -o$@ -lglfw -lGLEW -lGL $(CC_FILES)

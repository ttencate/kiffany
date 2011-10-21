cmake_minimum_required(VERSION 2.6)

project(kiffany)

# Allow inclusion of headers starting from the root of the source tree,
# not from the directory that the including file happens to be in.
include_directories(.)

set(GLFW_LIBRARY glfw)
set(GLEW_LIBRARY GLEW)

set(sources
	block.cc block.h
	camera.cc camera.h
	chunk.cc chunk.h
	chunkdata.cc chunkdata.h
	chunkmap.cc chunkmap.h
	coords.cc coords.h
	flags.cc flags.h
	gl.cc gl.h
	main.cc
	maths.cc maths.h
	octree.cc octree.h
	perlin.cc perlin.h
	queues.cc queues.h
	raycaster.cc raycaster.h
	stats.cc stats.h
	terrain.cc terrain.h
	terragen.cc terragen.h
	threading.cc threading.h
	threadpool.cc threadpool.h
	world.cc world.h
	)

add_executable(kiffany ${sources})

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_DETAILED_FAILURE_MSG TRUE)

find_package(Boost 1.47.0 REQUIRED COMPONENTS program_options system thread)
find_package(OpenGL REQUIRED)

target_link_libraries(kiffany
	${Boost_LIBRARIES}
	${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY}
	rt
	)
include_directories(
	${Boost_INCLUDE_DIRS}
	)

if (EXISTS /usr/bin/colorgcc)
	set(CMAKE_CXX_COMPILER /usr/bin/colorgcc)
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -ansi -pedantic -Wall -Wextra -Wno-long-long -Wno-unused-parameter -Wno-type-limits")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O2 -g -DASSERTIONS -DDEBUG=1")
set(CMAKE_CXX_FLAGS_PROFILE "-O3 -pg")

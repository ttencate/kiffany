cmake_minimum_required(VERSION 2.6)

project(kiffany)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -ansi -pedantic -Wall -Wextra -Wno-long-long -Wno-unused-parameter -Wno-type-limits")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O2 -g -DASSERTIONS -DDEBUG=1")
set(CMAKE_CXX_FLAGS_PROFILE "-O3 -pg")

# Allow inclusion of headers starting from the root of the source tree,
# not from the directory that the including file happens to be in.
include_directories(
	${Boost_INCLUDE_DIRS}
	.
	)

find_package(Boost 1.46.0 REQUIRED COMPONENTS program_options system thread)
find_package(OpenGL REQUIRED)

set(GLFW_LIBRARY glfw)
set(GLEW_LIBRARY GLEW)

set(sources
	atmosphere.cc atmosphere.h
	block.cc block.h
	buffer.cc buffer.h
	camera.cc camera.h
	chunk.cc chunk.h
	chunkdata.cc chunkdata.h
	chunkmanager.cc chunkmanager.h
	chunkmap.cc chunkmap.h
	coords.cc coords.h
	flags.cc flags.h
	geometry.cc geometry.h
	gl.cc gl.h
	lighting.cc lighting.h
	maths.cc maths.h
	octree.cc octree.h
	perlin.cc perlin.h
	raycaster.cc raycaster.h
	shader.cc shader.h
	sky.cc sky.h
	space.cc space.h
	stats.cc stats.h
	table.cc table.h
	terrain.cc terrain.h
	terragen.cc terragen.h
	threading.cc threading.h
	threadpool.cc threadpool.h
	world.cc world.h
	)

add_executable(kiffany ${sources} main.cc)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_DETAILED_FAILURE_MSG TRUE)

target_link_libraries(kiffany
	${Boost_LIBRARIES}
	${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY}
	rt
	Xrandr
	)
include_directories(
	${Boost_INCLUDE_DIRS}
	)

set(test_sources
	atmosphere_test.cc
	octree_test.cc
	perlin_test.cc
	raycaster_test.cc
	table_test.cc
	)

add_executable(kiffany_test ${sources} ${test_sources} main_test.cc)

target_link_libraries(kiffany_test
	${Boost_LIBRARIES}
	${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY}
	boost_unit_test_framework-mt.a
	)

add_executable(benchmark ${sources} benchmark.cc)

target_link_libraries(benchmark
	${Boost_LIBRARIES}
	${OPENGL_LIBRARY} ${GLFW_LIBRARY} ${GLEW_LIBRARY}
	)

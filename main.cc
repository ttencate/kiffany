#include "world.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <GL/glew.h>
#include <GL/glfw.h>
#include <iostream>
#include <cstdlib>

glm::mat4 const IDENTITY;

World *world = 0;
glm::mat4 projectionMatrix;
glm::mat4 viewMatrix;

bool running = true;

void keyCallback(int key, int state) {
	if (state == GLFW_PRESS) {
		switch (key) {
			case 'Q':
				running = false;
				break;
		}
	}
}

void setup() {
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glm::mat4 translation = glm::translate(IDENTITY, glm::vec3(0, 128, 0));
	glm::mat4 rotation = glm::rotate(IDENTITY, 90.0f, glm::vec3(-1, 0, 0));
	viewMatrix = rotation * translation;

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float lightPos[] = { 1, 1, 1, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
}

void update(float dt) {
	float const s = 30.0f * dt;
	if (glfwGetKey('O')) {
		viewMatrix = glm::translate(viewMatrix, glm::vec3(s, 0, 0));
	}
	if (glfwGetKey('U')) {
		viewMatrix = glm::translate(viewMatrix, glm::vec3(-s, 0, 0));
	}
	if (glfwGetKey('E')) {
		viewMatrix = glm::translate(viewMatrix, glm::vec3(0, s, 0));
	}
	if (glfwGetKey('.')) {
		viewMatrix = glm::translate(viewMatrix, glm::vec3(0, -s, 0));
	}
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int width, height;
	glfwGetWindowSize(&width, &height);
	glViewport(0, 0, width, height);
	projectionMatrix = glm::perspective(45.0f, (float)width / height, 0.1f, 1000.0f);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(projectionMatrix));

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(viewMatrix));

	world->render();
}

int main(int argc, char **argv) {
	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	glfwOpenWindow(1024, 768, 8, 8, 8, 8, 16, 0, GLFW_WINDOW);
	glfwSwapInterval(1); // vsync
	glfwSetWindowTitle("Kiffany");

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(err) << "\n";
		return EXIT_FAILURE;
	}
	if (!GLEW_VERSION_2_1) {
		std::cerr << "Error: OpenGL 2.1 not supported\n";
		return EXIT_FAILURE;
	}

	World world;
	::world = &world;

	glfwSetKeyCallback(keyCallback);

	setup();
	timespec lastUpdate;
	clock_gettime(CLOCK_MONOTONIC, &lastUpdate);
	while (running && glfwGetWindowParam(GLFW_OPENED)) {
		timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		float dt = (now.tv_sec - lastUpdate.tv_sec) + 1e-9 * (now.tv_nsec - lastUpdate.tv_nsec);
		lastUpdate = now;

		update(dt);
		render();
		glfwSwapBuffers();
	}

	glfwTerminate();
}

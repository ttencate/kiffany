#include "world.h"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include <GL/glew.h>
#include <GL/glfw.h>
#include <iostream>
#include <cstdlib>

glm::vec3 const X_AXIS(1, 0, 0);
glm::vec3 const Y_AXIS(0, 1, 0);
glm::vec3 const Z_AXIS(0, 0, 1);

World *world = 0;
glm::vec3 cameraPosition(0, -128, 0);
float cameraAzimuth = 0;
float cameraElevation = 0;

glm::int2 mousePos;
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

void mousePosCallback(int x, int y) {
	glm::int2 newMousePos(x, y);
	glm::int2 delta = newMousePos - mousePos;

	float const s = 0.5f;
	cameraAzimuth += s * -delta.x;
	cameraElevation += s * -delta.y;
	cameraElevation = glm::clamp(cameraElevation, -90.0f, 90.0f);

	mousePos = newMousePos;
}

void setup() {
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void update(float dt) {
	float const s = 30.0f * dt;
	glm::mat3 cameraRotation = glm::mat3(glm::rotate(cameraAzimuth, Z_AXIS) * glm::rotate(cameraElevation, X_AXIS));
	if (glfwGetKey('O')) {
		cameraPosition += cameraRotation * (s * -X_AXIS);
	}
	if (glfwGetKey('U')) {
		cameraPosition += cameraRotation * (s * X_AXIS);
	}
	if (glfwGetKey('E')) {
		cameraPosition += cameraRotation * (s * -Y_AXIS);
	}
	if (glfwGetKey('.')) {
		cameraPosition += cameraRotation * (s * Y_AXIS);
	}
	if (glfwGetKey(GLFW_KEY_LSHIFT)) {
		cameraPosition += cameraRotation * (s * -Z_AXIS);
	}
	if (glfwGetKey(' ')) {
		cameraPosition += cameraRotation * (s * Z_AXIS);
	}
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int width, height;
	glfwGetWindowSize(&width, &height);
	glViewport(0, 0, width, height);
	glm::mat4 projectionMatrix = glm::perspective(45.0f, (float)width / height, 0.1f, 1000.0f);

	glm::mat4 rotateCoords = glm::rotate(-90.0f, X_AXIS);
	glm::mat4 translate = glm::translate(-cameraPosition);
	glm::mat4 rotateZ = glm::rotate(-cameraAzimuth, Z_AXIS);
	glm::mat4 rotateX = glm::rotate(-cameraElevation, X_AXIS);
	glm::mat4 viewMatrix = rotateCoords * rotateX * rotateZ * translate;

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(projectionMatrix));

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(viewMatrix));

	float lightPos[] = { 1, 2, 3, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

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
	glfwSetMousePosCallback(mousePosCallback);
	glfwDisable(GLFW_MOUSE_CURSOR);
	glfwGetMousePos(&mousePos[0], &mousePos[1]);

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

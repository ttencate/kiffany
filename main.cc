#include "world.h"

#include <GL/glew.h>
#include <GL/glfw.h>
#include <iostream>
#include <cstdlib>

World *world = 0;

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

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(90, -1, 0, 0);
	glTranslatef(0, 128, 0);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	float lightPos[] = { 1, 1, 1, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
}

void update() {
	float const s = 1.0;
	if (glfwGetKey('O')) {
		glTranslatef(s, 0, 0);
	}
	if (glfwGetKey('U')) {
		glTranslatef(-s, 0, 0);
	}
	if (glfwGetKey('.')) {
		glTranslatef(0, -s, 0);
	}
	if (glfwGetKey('E')) {
		glTranslatef(0, s, 0);
	}
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int width, height;
	glfwGetWindowSize(&width, &height);
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (double)width / height, 0.1, 1000);
	glMatrixMode(GL_MODELVIEW);

	world->render();
}

int main(int argc, char **argv) {
	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	glfwOpenWindow(1024,768, 8, 8, 8, 8, 16, 0, GLFW_WINDOW);
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
	while (running && glfwGetWindowParam(GLFW_OPENED)) {
		update();
		render();
		glfwSwapBuffers();
	}

	glfwTerminate();
}

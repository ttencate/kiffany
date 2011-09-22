#include "world.h"

#include <GL/glfw.h>
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

void update() {
}

void render() {
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	world->render();
}

int main(int argc, char **argv) {
	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	World world;
	::world = &world;

	glfwOpenWindow(1024,768, 8, 8, 8, 8, 16, 0, GLFW_WINDOW);
	glfwSwapInterval(1); // vsync
	glfwSetWindowTitle("Kiffany");
	glfwSetKeyCallback(keyCallback);

	while (running && glfwGetWindowParam(GLFW_OPENED)) {
		update();
		render();
		glfwSwapBuffers();
	}
	glfwTerminate();
}

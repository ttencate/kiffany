#include "camera.h"
#include "flags.h"
#include "maths.h"
#include "stats.h"
#include "terragen.h"
#include "world.h"

#include <iostream>
#include <cstdlib>

World *world = 0;
Camera *camera = 0;

int2 mousePos;
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
	if (!flags.autofly) {
		int2 newMousePos(x, y);
		int2 delta = newMousePos - mousePos;

		float const s = 0.5f;
		camera->setAzimuth(camera->getAzimuth() + s * -delta.x);
		camera->setElevation(camera->getElevation() + s * -delta.y);

		mousePos = newMousePos;
	}
}

void setup() {
	glClearColor(0.7f, 0.8f, 1.0f, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void update(float dt) {
	if (flags.autofly) {
		float const s = 30.0f * dt;
		camera->moveRelative(s * Y_AXIS);
	} else {
		float const s = 5.0f * dt;
		vec3 delta;
		if (glfwGetKey('O')) {
			delta += s * -X_AXIS;
		}
		if (glfwGetKey('U')) {
			delta += s * X_AXIS;
		}
		if (glfwGetKey('E')) {
			delta += s * -Y_AXIS;
		}
		if (glfwGetKey('.')) {
			delta += s * Y_AXIS;
		}
		if (glfwGetKey(GLFW_KEY_LSHIFT)) {
			delta += s * -Z_AXIS;
		}
		if (glfwGetKey(' ')) {
			delta += s * Z_AXIS;
		}
		camera->moveRelative(delta);
	}

	world->update(dt);
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int width, height;
	glfwGetWindowSize(&width, &height);
	glViewport(0, 0, width, height);
	mat4 projectionMatrix = perspective(45.0f, (float)width / height, 0.1f, 1000.0f);

	mat4 rotateCoords = rotate(-90.0f, X_AXIS);
	mat4 viewMatrix = rotateCoords * camera->getMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(value_ptr(projectionMatrix));

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(value_ptr(viewMatrix));

	float lightPos[] = { 1, 2, 3, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	world->render();
}

int main(int argc, char **argv) {
	if (!parseCommandLine(argc, argv)) {
		return EXIT_FAILURE;
	}

	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	glfwOpenWindow(1024, 768, 8, 8, 8, 8, 16, 0, GLFW_WINDOW);
	glfwSwapInterval(flags.vsync ? 1 : 0);
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

	Camera camera;
	camera.setPosition(vec3(0.0f, 0.0f, 16.0f));
	::camera = &camera;
	World world(&camera, new SineTerrainGenerator()); //PerlinTerrainGenerator(32, flags.seed));
	::world = &world;

	glfwSetKeyCallback(keyCallback);
	glfwSetMousePosCallback(mousePosCallback);
	glfwDisable(GLFW_MOUSE_CURSOR);
	glfwGetMousePos(&mousePos[0], &mousePos[1]);

	setup();

	{
		Timed t = stats.runningTime.timed();
		timespec lastUpdate;
		clock_gettime(CLOCK_MONOTONIC, &lastUpdate);
		while (running && glfwGetWindowParam(GLFW_OPENED)) {
			float dt;
			if (flags.fixedTimestep) {
				dt = 1e-3f * flags.fixedTimestep;
			} else {
				timespec now;
				clock_gettime(CLOCK_MONOTONIC, &now);
				dt = (now.tv_sec - lastUpdate.tv_sec) + 1e-9 * (now.tv_nsec - lastUpdate.tv_nsec);
				lastUpdate = now;
				if (dt > 0.1f) {
					dt = 0.1f;
				}
			}

			update(dt);
			render();
			stats.framesRendered.increment();
			glfwSwapBuffers();

			if (flags.exitAfter && stats.framesRendered.get() >= flags.exitAfter) {
				break;
			}
		}
	}

	stats.print();
	glfwTerminate();
}

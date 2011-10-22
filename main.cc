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

bool mouseLook;
int2 mousePos;
bool running = true;

void setMouseLook(bool mouseLook) {
	::mouseLook = mouseLook;
	if (mouseLook) {
		glfwGetMousePos(&mousePos[0], &mousePos[1]);
		glfwDisable(GLFW_MOUSE_CURSOR);
	} else {
		glfwEnable(GLFW_MOUSE_CURSOR);
	}
}

void GLFWCALL keyCallback(int key, int state) {
	if (state == GLFW_PRESS) {
		switch (key) {
			case 'Q':
				running = false;
				break;
			case 'S':
				stats.print();
				std::cout << "----------------------------------------\n";
				break;
		}
	}
}

void GLFWCALL mouseButtonCallback(int button, int action) {
	if (action == GLFW_PRESS) {
		switch (button) {
			case GLFW_MOUSE_BUTTON_LEFT:
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				setMouseLook(!mouseLook);
				break;
		}
	}
}

void GLFWCALL mousePosCallback(int x, int y) {
	if (mouseLook) {
		int2 newMousePos(x, y);
		int2 delta = newMousePos - mousePos;

		float const s = 0.5f;
		camera->setAzimuth(camera->getAzimuth() + s * -delta.x);
		camera->setElevation(camera->getElevation() + s * -delta.y);

		mousePos = newMousePos;
	}
}

void GLFWCALL windowSizeCallback(int width, int height) {
	glViewport(0, 0, width, height);
	mat4 projectionMatrix = perspective(45.0f, (float)width / height, 0.1f, 1000.0f);
	camera->setProjectionMatrix(projectionMatrix);
}

void setup() {
	glClearColor(0.7f, 0.8f, 1.0f, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void update(float dt) {
	if (flags.autoflySpeed != 0) {
		float const s = flags.autoflySpeed * dt;
		camera->moveRelative(s * Y_AXIS);
	} else {
		float const s = 100.0f * dt;
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
		camera->moveRelative(delta);

		if (glfwGetKey(GLFW_KEY_LSHIFT)) {
			camera->setPosition(camera->getPosition() + s * -Z_AXIS);
		}
		if (glfwGetKey(' ')) {
			camera->setPosition(camera->getPosition() + s * Z_AXIS);
		}
	}

	world->update(dt);
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(value_ptr(camera->getProjectionMatrix()));
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(value_ptr(camera->getViewMatrix()));

	float lightPos[] = { 1, 2, 3, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

	world->render();
}

void run() {
	glfwSetWindowSizeCallback(windowSizeCallback); // also calls it immediately
	glfwSetKeyCallback(keyCallback);
	glfwSetMouseButtonCallback(mouseButtonCallback);
	glfwSetMousePosCallback(mousePosCallback);
	setMouseLook(flags.mouseLook);

	setup();

	{
		timespec lastUpdate;
		clock_gettime(CLOCK_MONOTONIC, &lastUpdate);
		while (running && glfwGetWindowParam(GLFW_OPENED)) {
			UnsafeTimer::Timed t = stats.runningTime.timed();

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
}

int main(int argc, char **argv) {
	if (!parseCommandLine(argc, argv)) {
		return EXIT_FAILURE;
	}

	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	Camera camera;
	camera.setPosition(vec3(flags.startX, flags.startY, flags.startZ));
	::camera = &camera;
	World world(&camera, new PerlinTerrainGenerator(32, flags.seed));
	::world = &world;

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

	run();

	stats.print();
	glfwTerminate();
}

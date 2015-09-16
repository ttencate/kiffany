#include "camera.h"
#include "flags.h"
#include "maths.h"
#include "raycaster.h"
#include "stats.h"
#include "terragen.h"
#include "world.h"

#include <GLFW/glfw3.h>

#include <iostream>
#include <cstdlib>

World *world = 0;
Camera *camera = 0;

GLFWwindow *window;
bool mouseLook;
int2 cursorPos;
bool fullscreen;
bool running = true;

struct Marker {
	vec3 position;
	void render() {
		float const s = 0.5f;
		float const a = 1024.0f;

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);	

		glDisable(GL_LIGHTING);
		glColor3f(1.0f, 0.0f, 0.0f);
		glPushMatrix();
		glTranslatef(position.x, position.y, position.z);

		glLineWidth(2.0f);
		glBegin(GL_LINES);
		glVertex3f(-s, -s, -s);
		glVertex3f(s, s, s);
		glVertex3f(s, -s, -s);
		glVertex3f(-s, s, s);
		glVertex3f(-s, s, -s);
		glVertex3f(s, -s, s);
		glVertex3f(s, s, -s);
		glVertex3f(-s, -s, s);
		glEnd();

		glLineWidth(1.0f);
		glBegin(GL_LINES);
		glVertex3f(-a, 0, 0);
		glVertex3f(a, 0, 0);
		glVertex3f(0, -a, 0);
		glVertex3f(0, a, 0);
		glVertex3f(0, 0, -a);
		glVertex3f(0, 0, a);
		glEnd();

		glPopMatrix();
		glEnable(GL_LIGHTING);
	}
};

Marker marker;

void testRaycast() {
	int3 chunkIndex = chunkIndexFromPoint(camera->getPosition()); // TODO use chunk-relative in camera
	vec3 origin = camera->getPosition() - chunkMin(chunkIndex);
	vec3 direction = camera->getFrontVector();

	Raycaster raycaster(world->getTerrain().getChunkMap(), 1024.0f, STONE_BLOCK);
	RaycastResult result = raycaster(chunkIndex, origin, direction);

	marker.position = chunkMin(chunkIndex) + result.endPointInStartChunk;
}

void setMouseLook(bool mouseLook) {
	::mouseLook = mouseLook;
	if (mouseLook) {
    double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
    cursorPos[0] = (int) mouseX;
    cursorPos[1] = (int) mouseY;
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	} else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
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

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
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

void cursorPosCallback(GLFWwindow *window, double x, double y) {
	if (mouseLook) {
		int2 newCursorPos((int) x, (int) y);
		int2 delta = newCursorPos - cursorPos;

		float const s = 0.5f;
		camera->setAzimuth(camera->getAzimuth() + s * -delta.x);
		camera->setElevation(camera->getElevation() + s * -delta.y);

		cursorPos = newCursorPos;
	}
}

void windowSizeCallback(GLFWwindow *window, int width, int height) {
	glViewport(0, 0, width, height);
	mat4 projectionMatrix = perspective(45.0f, (float)width / height, 0.1f, 1000.0f);
	camera->setProjectionMatrix(projectionMatrix);
}

void setup() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void update(float dt) {
	if (flags.autoflySpeed != 0) {
		float const s = flags.autoflySpeed * dt;
		camera->moveRelative(s * Y_AXIS);
	} else {
		float const acc = 200.0f;
		float const minV = 5.0f;
		float const maxV = 100.0f;
		static float v = 0.0f;
		vec3 relDelta;
		vec3 absDelta;
		bool pressed = false;

		if (glfwGetKey(window, 'O')) {
			relDelta += -X_AXIS;
			pressed = true;
		}
		if (glfwGetKey(window, 'U')) {
			relDelta += X_AXIS;
			pressed = true;
		}
		if (glfwGetKey(window, 'E')) {
			relDelta += -Y_AXIS;
			pressed = true;
		}
		if (glfwGetKey(window, '.')) {
			relDelta += Y_AXIS;
			pressed = true;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
			absDelta -= Z_AXIS;
			pressed = true;
		}
		if (glfwGetKey(window, ' ')) {
			absDelta += Z_AXIS;
			pressed = true;
		}

		if (!pressed) {
			v = 0.0f;
		} else {
			v = std::min(maxV, std::max(minV, v + acc * dt));
		}
		float d = v * dt;

		camera->moveRelative(d * relDelta);
		camera->setPosition(camera->getPosition() + d * absDelta);
	}

	world->update(dt);
	// testRaycast();
}

void render() {
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(value_ptr(camera->getProjectionMatrix()));

	world->render();

	// marker.render();
}

void run() {
	glfwSetWindowSizeCallback(window, windowSizeCallback); // also calls it immediately
	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	setMouseLook(flags.mouseLook);

	setup();

	{
		timespec lastUpdate;
		clock_gettime(CLOCK_MONOTONIC, &lastUpdate);
		while (running && !glfwWindowShouldClose(window)) {
			TimerStat::Timed t = stats.runningTime.timed();

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
			glfwSwapBuffers(window);

			if (flags.exitAfter && stats.framesRendered.get() >= flags.exitAfter) {
				break;
			}
		}
	}
}

void errorCallback(int error, char const *description) {
  std::cerr << "GLFW error " << error << ": " << description << '\n';
}

int main(int argc, char **argv) {
	if (!parseCommandLine(argc, argv)) {
		return EXIT_FAILURE;
	}
	if (flags.help) {
		printHelp();
		return EXIT_SUCCESS;
	}

  glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) {
		return EXIT_FAILURE;
	}

	GLFWmonitor *monitor;
	int width;
	int height;
	if (flags.fullscreen) {
		monitor = glfwGetPrimaryMonitor();
		GLFWvidmode const *videoMode = glfwGetVideoMode(monitor);
    if (!videoMode) {
      return EXIT_FAILURE;
    }
		width = videoMode->width;
		height = videoMode->height;
	} else {
		monitor = NULL;
		width = 1024;
		height = 768;
	}
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	window = glfwCreateWindow(width, height, "Kiffany", monitor, NULL);
  if (!window) {
    return EXIT_FAILURE;
  }

  glfwMakeContextCurrent(window);
	glfwSwapInterval(flags.vsync ? 1 : 0);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "GLEW error: " << glewGetErrorString(err) << "\n";
		return EXIT_FAILURE;
	}
	if (!GLEW_VERSION_1_4) {
		std::cerr << "GLEW error: OpenGL 2.0 not supported\n";
		return EXIT_FAILURE;
	}

	Camera camera;
	camera.setPosition(vec3(flags.startX, flags.startY, flags.startZ));
	::camera = &camera;

	// TODO make owned by world, do not leak
	GLAtmosphere *atmosphere = new GLAtmosphere(AtmosParams());
	Sun *sun = new Sun(
			(flags.dayOfYear - 1.0f) / 365.0f,
			radians(flags.latitude),
			radians(flags.axialTilt),
			flags.dayLength,
			radians(flags.sunAngularDiameter / 2.0f),
			flags.sunBrightness,
			vec3(1.0f),
			flags.startTime / 24.0f);
	World world(
			&camera,
			new PerlinTerrainGenerator(32, flags.seed),
			sun,
			new Lighting(atmosphere, sun),
			new Sky(atmosphere, sun));
	::world = &world;

	run();

	stats.print();
	glfwTerminate();

	return EXIT_SUCCESS;
}

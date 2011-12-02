#include "camera.h"
#include "flags.h"
#include "maths.h"
#include "raycaster.h"
#include "stats.h"
#include "terragen.h"
#include "world.h"

#include <GL/glfw.h>

#include <iostream>
#include <cstdlib>

World *world = 0;
Camera *camera = 0;

bool mouseLook;
int2 mousePos;
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

		if (glfwGetKey('O')) {
			relDelta += -X_AXIS;
			pressed = true;
		}
		if (glfwGetKey('U')) {
			relDelta += X_AXIS;
			pressed = true;
		}
		if (glfwGetKey('E')) {
			relDelta += -Y_AXIS;
			pressed = true;
		}
		if (glfwGetKey('.')) {
			relDelta += Y_AXIS;
			pressed = true;
		}
		if (glfwGetKey(GLFW_KEY_LSHIFT)) {
			absDelta -= Z_AXIS;
			pressed = true;
		}
		if (glfwGetKey(' ')) {
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

	int width;
	int height;
	int mode;
	if (flags.fullscreen) {
		GLFWvidmode videoMode;
		glfwGetDesktopMode(&videoMode);
		width = videoMode.Width;
		height = videoMode.Height;
		mode = GLFW_FULLSCREEN;
	} else {
		width = 1024;
		height = 768;
		mode = GLFW_WINDOW;
	}
	glfwOpenWindow(width, height, 8, 8, 8, 8, 16, 0, mode);
	glfwSwapInterval(flags.vsync ? 1 : 0);
	glfwSetWindowTitle("Kiffany");

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cerr << "Error: " << glewGetErrorString(err) << "\n";
		return EXIT_FAILURE;
	}
	if (!GLEW_VERSION_3_0) {
		std::cerr << "Error: OpenGL 3.0 not supported\n";
		return EXIT_FAILURE;
	}

	Camera camera;
	camera.setPosition(vec3(flags.startX, flags.startY, flags.startZ));
	::camera = &camera;

	Atmosphere atmosphere;
	Sun *sun = new Sun(
			(flags.dayOfYear - 1.0f) / 365.0f,
			radians(flags.latitude),
			radians(flags.axialTilt),
			flags.dayLength,
			radians(flags.sunAngularDiameter / 2.0f),
			flags.sunBrightness * vec3(1.0f),
			flags.startTime / 24.0f);
	World world(
			&camera,
			new PerlinTerrainGenerator(32, flags.seed),
			sun,
			new Lighting(sun),
			new Sky(atmosphere, AtmosphereLayers(atmosphere, flags.atmosphereLayers, flags.atmosphereAngles), sun));
	::world = &world;

	run();

	stats.print();
	glfwTerminate();

	return EXIT_SUCCESS;
}

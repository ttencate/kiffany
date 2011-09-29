#include "camera.h"

Camera::Camera()
:
	azimuth(0),
	elevation(0)
{
	update();
}

void Camera::setProjectionMatrix(mat4 const &projectionMatrix) {
	this->projectionMatrix = projectionMatrix;
	update();
}

void Camera::setPosition(vec3 const &position) {
	this->position = position;
	update();
}

void Camera::setAzimuth(float azimuth) {
	this->azimuth = azimuth;
	update();
}

void Camera::setElevation(float elevation) {
	this->elevation = clamp(elevation, -90.0f, 90.0f);
	update();
}

void Camera::moveRelative(vec3 const &delta) {
	mat3 rotation = mat3(rotate(azimuth, Z_AXIS) * rotate(elevation, X_AXIS));
	position += rotation * delta;
	update();
}

bool Camera::isSphereInView(vec3 const &center, float radius) const {
	vec4 const p = vec4(center, 1.0f);
	for (unsigned i = 0; i < 6; ++i) {
		if (dot(p, frustum[i]) <= -radius) {
			return false;
		}
	}
	return true;
}

void Camera::update() {
	mat4 translation = translate(-position);
	mat4 rotationZ = rotate(-azimuth, Z_AXIS);
	mat4 rotationX = rotate(-elevation, X_AXIS);
	mat4 rotateCoords = rotate(-90.0f, X_AXIS);
	viewMatrix = rotateCoords * rotationX * rotationZ * translation;

	viewProjectionMatrix = projectionMatrix * viewMatrix;

	viewProjectionMatrixInverse = inverse(viewProjectionMatrix);

	mat4 const m = transpose(viewProjectionMatrix);
	frustum[0] = m[3] - m[0];
	frustum[1] = m[3] + m[0];
	frustum[2] = m[3] - m[1];
	frustum[3] = m[3] + m[1];
	frustum[4] = m[3] - m[2];
	frustum[5] = m[3] + m[2];
	for (unsigned i = 0; i < 6; ++i) {
		frustum[i] /= length(vec3(frustum[i]));
	}
}

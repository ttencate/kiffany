#include "camera.h"

Camera::Camera()
:
	azimuth(0),
	elevation(0)
{
	updateViewMatrix();
	updateViewProjectionMatrix();
}

void Camera::setProjectionMatrix(mat4 const &projectionMatrix) {
	this->projectionMatrix = projectionMatrix;
	updateViewProjectionMatrix();
}

void Camera::setPosition(vec3 const &position) {
	this->position = position;
	updateViewMatrix();
}

void Camera::setAzimuth(float azimuth) {
	this->azimuth = azimuth;
	updateViewMatrix();
}

void Camera::setElevation(float elevation) {
	this->elevation = clamp(elevation, -90.0f, 90.0f);
	updateViewMatrix();
}

void Camera::moveRelative(vec3 const &delta) {
	mat3 rotation = mat3(rotate(azimuth, Z_AXIS) * rotate(elevation, X_AXIS));
	position += rotation * delta;
	updateViewMatrix();
}

bool Camera::isInView(vec3 const &point) const {
	vec4 clipPoint = viewProjectionMatrix * vec4(point, 1.0f);
	vec3 p = vec3(clipPoint) / clipPoint.w;
	return
		p.x >= -1 && p.x <= 1 &&
		p.y >= -1 && p.y <= 1 &&
		p.z >= 0 && p.z <= 1;
}

void Camera::updateViewMatrix() {
	mat4 translation = translate(-position);
	mat4 rotationZ = rotate(-azimuth, Z_AXIS);
	mat4 rotationX = rotate(-elevation, X_AXIS);
	mat4 rotateCoords = rotate(-90.0f, X_AXIS);
	viewMatrix = rotateCoords * rotationX * rotationZ * translation;
	updateViewProjectionMatrix();
}

void Camera::updateViewProjectionMatrix() {
	viewProjectionMatrix = projectionMatrix * viewMatrix;
}

#include "camera.h"

Camera::Camera()
:
	azimuth(0),
	elevation(0)
{
}

void Camera::setPosition(vec3 const &position) {
	this->position = position;
}

void Camera::setAzimuth(float azimuth) {
	this->azimuth = azimuth;
}

void Camera::setElevation(float elevation) {
	this->elevation = clamp(elevation, -90.0f, 90.0f);
}

mat4 Camera::getMatrix() const {
	mat4 translation = translate(-position);
	mat4 rotationZ = rotate(-azimuth, Z_AXIS);
	mat4 rotationX = rotate(-elevation, X_AXIS);
	return rotationX * rotationZ * translation;
}

void Camera::moveRelative(vec3 delta) {
	mat3 rotation = mat3(rotate(azimuth, Z_AXIS) * rotate(elevation, X_AXIS));
	position += rotation * delta;
}

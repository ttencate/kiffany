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

bool isTransformedPointInBox(vec3 const &point, mat4 const &transform, vec3 const &min, vec3 const &max) {
	vec4 transformedPoint = transform * vec4(point, 1.0f);
	vec3 p = vec3(transformedPoint) / transformedPoint.w;
	return 
		p.x >= min.x && p.x <= max.x &&
		p.y >= min.y && p.y <= max.y &&
		p.z >= min.z && p.z <= max.z;
}

bool isTransformedBoxInBox(vec3 const &minA, vec3 const &maxA, mat4 const &transform, vec3 const &minB, vec3 const &maxB) {
	return
		isTransformedPointInBox(vec3(minA.x, minA.y, minA.z), transform, minB, maxB) ||
		isTransformedPointInBox(vec3(maxA.x, minA.y, minA.z), transform, minB, maxB) ||
		isTransformedPointInBox(vec3(minA.x, maxA.y, minA.z), transform, minB, maxB) ||
		isTransformedPointInBox(vec3(maxA.x, maxA.y, minA.z), transform, minB, maxB) ||
		isTransformedPointInBox(vec3(minA.x, minA.y, maxA.z), transform, minB, maxB) ||
		isTransformedPointInBox(vec3(maxA.x, minA.y, maxA.z), transform, minB, maxB) ||
		isTransformedPointInBox(vec3(minA.x, maxA.y, maxA.z), transform, minB, maxB) ||
		isTransformedPointInBox(vec3(maxA.x, maxA.y, maxA.z), transform, minB, maxB);
}

bool Camera::isBoxInView(vec3 const &min, vec3 const &max) const {
	vec3 clipMin(-1, -1, 0);
	vec3 clipMax( 1,  1, 1);
	return
		isTransformedBoxInBox(min, max, viewProjectionMatrix, clipMin, clipMax) ||
		isTransformedBoxInBox(clipMin, clipMax, viewProjectionMatrixInverse, min, max);
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
	viewProjectionMatrixInverse = inverse(viewProjectionMatrix);
}

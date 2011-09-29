#ifndef CAMERA_H
#define CAMERA_H

#include "maths.h"

class Camera {

	mat4 projectionMatrix;

	vec3 position;
	float azimuth;
	float elevation;

	public:

		Camera();

		mat4 getProjectionMatrix() const { return projectionMatrix; }
		void setProjectionMatrix(mat4 const &projectionMatrix);

		vec3 const &getPosition() const { return position; }
		void setPosition(vec3 const &position);

		float getAzimuth() const { return azimuth; }
		void setAzimuth(float azimuth);

		float getElevation() const { return elevation; }
		void setElevation(float elevation);

		mat4 getViewMatrix() const;

		void moveRelative(vec3 delta);

};

#endif

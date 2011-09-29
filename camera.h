#ifndef CAMERA_H
#define CAMERA_H

#include "maths.h"

class Camera {

	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 viewProjectionMatrix;

	vec3 position;
	float azimuth;
	float elevation;

	public:

		Camera();

		mat4 const &getProjectionMatrix() const { return projectionMatrix; }
		void setProjectionMatrix(mat4 const &projectionMatrix);

		vec3 const &getPosition() const { return position; }
		void setPosition(vec3 const &position);

		float getAzimuth() const { return azimuth; }
		void setAzimuth(float azimuth);

		float getElevation() const { return elevation; }
		void setElevation(float elevation);

		mat4 const &getViewMatrix() const { return viewMatrix; }

		void moveRelative(vec3 const &delta);

		bool isInView(vec3 const &point) const;

	private:

		void updateViewMatrix();
		void updateViewProjectionMatrix();

};

#endif

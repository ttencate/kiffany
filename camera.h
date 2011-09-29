#ifndef CAMERA_H
#define CAMERA_H

#include "maths.h"

class Camera {

	mat4 projectionMatrix;
	mat4 viewMatrix;

	mat4 viewProjectionMatrix;
	mat4 viewProjectionMatrixInverse;
	vec4 frustum[6];

	vec3 position;
	float azimuth;
	float elevation;

	public:

		Camera();

		void setProjectionMatrix(mat4 const &projectionMatrix);

		vec3 const &getPosition() const { return position; }
		void setPosition(vec3 const &position);

		float getAzimuth() const { return azimuth; }
		void setAzimuth(float azimuth);

		float getElevation() const { return elevation; }
		void setElevation(float elevation);

		void moveRelative(vec3 const &delta);

		mat4 const &getProjectionMatrix() const { return projectionMatrix; }
		mat4 const &getViewMatrix() const { return viewMatrix; }

		bool isSphereInView(vec3 const &center, float radius) const;

	private:

		void update();

};

#endif

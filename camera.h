#ifndef CAMERA_H
#define CAMERA_H

#include "maths.h"

class Camera {

	vec3 position;
	float azimuth;
	float elevation;

	public:

		Camera();

		vec3 const &getPosition() const { return position; }
		void setPosition(vec3 const &position);

		float getAzimuth() const { return azimuth; }
		void setAzimuth(float azimuth);

		float getElevation() const { return elevation; }
		void setElevation(float elevation);

		mat4 getMatrix() const;

		void moveRelative(vec3 delta);

};

#endif

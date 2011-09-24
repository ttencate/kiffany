#ifndef MATHS_H
#define MATHS_H

#include "glm/glm.hpp"
#include "glm/ext.hpp"

using glm::int2;
using glm::int3;
using glm::int4;
using glm::mat2;
using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::clamp;
using glm::floor;
using glm::perspective;
using glm::translate;
using glm::rotate;
using glm::value_ptr;

extern vec3 const X_AXIS;
extern vec3 const Y_AXIS;
extern vec3 const Z_AXIS;

extern int3 const X_STEP;
extern int3 const Y_STEP;
extern int3 const Z_STEP;

#endif

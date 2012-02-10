#version 120

varying out vec3 normal;

void main() {
	gl_Position = ftransform();
	normal = gl_Normal;
}

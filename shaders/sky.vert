#version 120

varying out vec3 direction;

void main() {
	gl_Position = ftransform();
	direction = vec3(gl_Vertex);
}

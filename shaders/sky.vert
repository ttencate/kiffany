#version 120

varying out vec3 viewDirectionUnnormalized;

void main() {
	gl_Position = ftransform();
	viewDirectionUnnormalized = vec3(gl_Vertex);
}

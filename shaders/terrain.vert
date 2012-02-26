#version 120

uniform vec3 cameraPosition;
uniform vec3 vertexOffset;

varying out vec3 normal;
varying out vec3 viewRay;

void main() {
	gl_Position = ftransform();
	normal = gl_Normal;
	viewRay = vertexOffset + vec3(gl_Vertex) - cameraPosition;
}

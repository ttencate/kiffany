#version 150

struct Material {
	vec4 ambient;
	vec4 diffuse;
};

struct Lighting {
	vec4 ambientColor;
	vec4 sunColor;
	vec3 sunDirection;
};

uniform Material material;
uniform Lighting lighting;

in vec3 normal;

out vec4 color;

void main() {
	float diff = dot(normal, lighting.sunDirection);
	color = lighting.ambientColor * material.ambient + diff * lighting.sunColor * material.diffuse;
}

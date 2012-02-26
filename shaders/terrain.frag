#version 150

// TODO write shader preprocessor to avoid copying
// BEGIN copied from sky.frag
const float PI = 3.1415926535;

struct AtmosParams {
	float earthRadius;
	vec3 rayleighCoefficient;
	vec3 mieCoefficient;
	float mieDirectionality;
	int numLayers;
	int numAngles;
};

struct Sun {
	float angularRadius;
	vec3 color;
	vec3 direction;
};

uniform AtmosParams params;
uniform Sun sun;
uniform sampler2DRect totalTransmittanceSampler;

float pow2(float x) {
	return x * x;
}

float rayleighPhaseFunction(float lightAngle) {
	float mu = cos(lightAngle);
	return
		3.0 / (16.0 * PI)
		* (1.0 + pow2(mu));
}

float miePhaseFunction(float lightAngle) {
	float mu = cos(lightAngle);
	float g = params.mieDirectionality;
	return 3.0 / (8.0 * PI) *
		(1 - pow2(g)) * (1 + pow2(mu)) /
		((2 + pow2(g)) * pow(1 + pow2(g) - 2.0 * g * mu, 3.0 / 2.0));
}

vec3 sampleTable(sampler2DRect tableSampler, int layer, float angle) {
	return vec3(texture(tableSampler, vec2(
					layer + 0.5,
					angle / PI * (params.numAngles - 1) + 0.5)));
}
// END copied from sky.frag

struct Material {
	vec4 ambient;
	vec4 diffuse;
};

struct Lighting {
	vec4 ambientColor;
};

uniform Material material;
uniform Lighting lighting;

in vec3 normal;
in vec3 viewRay;

out vec4 color;

// TODO much of this can be moved to the vertex shader
void main() {
	// Ambient
	vec4 ambient = lighting.ambientColor * material.ambient;

	// Diffuse
	float sunAngle = acos(0.99999 * sun.direction.z);
	vec3 sunTransmittance = sampleTable(totalTransmittanceSampler, 0, sunAngle);
	vec4 diffuse = dot(normal, sun.direction) * vec4(sun.color * sunTransmittance, 1.0) * material.diffuse;

	// Inscattering
	float rayLength = length(viewRay);
	vec3 viewDirection = normalize(viewRay);
	float lightAngle = acos(dot(viewDirection, sun.direction));
	vec3 rayleighInscattering = params.rayleighCoefficient * rayleighPhaseFunction(lightAngle);
	vec3 mieInscattering = params.mieCoefficient * miePhaseFunction(lightAngle);
	vec3 inscattering = rayLength * sun.color * sunTransmittance * (rayleighInscattering + mieInscattering);

	color = ambient + diffuse + vec4(inscattering, 1.0);
}

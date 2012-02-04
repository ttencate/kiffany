#version 150

const float PI = 3.1415926535;

struct Ray {
	float height;
	float angle;
};

struct Atmosphere {
	float earthRadius;
	vec3 rayleighCoefficient;
	vec3 mieCoefficient;
	float mieDirectionality;
};

struct Sun {
	float angularRadius;
	vec3 color;
	vec3 direction;
};

struct Layers {
	int numLayers;
	int numAngles;
	float heights[32];
	float rayleighDensities[32];
	float mieDensities[32];
};

uniform Atmosphere atmosphere;
uniform Sun sun;
uniform Layers layers;
uniform sampler2DRect transmittanceSampler;
uniform sampler2DRect totalTransmittanceSampler;

in vec3 viewDirectionUnnormalized;

out vec3 scatteredLight;

float pow2(float x) {
	return x * x;
}

float rayAngleUpwards(Ray ray, float targetHeight) {
	return asin(ray.height * sin(ray.angle) / targetHeight);
}

float rayLengthUpwards(Ray ray, float targetHeight) {
	float cosAngle = cos(ray.angle);
	return sqrt(
			pow2(ray.height) * (pow2(cosAngle) - 1.0) +
			pow2(targetHeight))
		- ray.height * cosAngle;
}

float rayleighPhaseFunction(float lightAngle) {
	float mu = cos(lightAngle);
	return
		3.0 / (16.0 * PI)
		* (1.0 + pow2(mu));
}

float miePhaseFunction(float lightAngle) {
	float mu = cos(lightAngle);
	float g = atmosphere.mieDirectionality;
	return 3.0 / (8.0 * PI) *
		(1 - pow2(g)) * (1 + pow2(mu)) /
		((2 + pow2(g)) * pow(1 + pow2(g) - 2.0 * g * mu, 3.0 / 2.0));
}

vec3 sampleTable(sampler2DRect tableSampler, int layer, float angle) {
	return vec3(texture(tableSampler, vec2(
					layer + 0.5,
					angle / PI * (layers.numAngles - 1) + 0.5)));
}

void main() {
	if (viewDirectionUnnormalized.z < 0.0) {
		scatteredLight = vec3(0.0);
		return;
	}
	vec3 viewDirection = normalize(viewDirectionUnnormalized);

	float lightAngle = acos(dot(viewDirection, sun.direction));
	float groundViewAngle = acos(viewDirection.z);
	float groundSunAngle = acos(sun.direction.z);
	Ray groundViewRay = Ray(atmosphere.earthRadius, groundViewAngle);
	Ray groundSunRay = Ray(atmosphere.earthRadius, groundSunAngle);

	vec3 rayleighPhase = vec3(rayleighPhaseFunction(lightAngle));
	vec3 miePhase = vec3(miePhaseFunction(lightAngle));

	scatteredLight = (1.0 - smoothstep(sun.angularRadius, sun.angularRadius * 1.2, lightAngle)) * sun.color;
	for (int layer = layers.numLayers - 2; layer >= 0; --layer) {
		float height = layers.heights[layer];

		Ray viewRay = Ray(height, rayAngleUpwards(groundViewRay, height));
		float rayLength = rayLengthUpwards(viewRay, layers.heights[layer + 1]);

		vec3 vertical = normalize(vec3(0.0, 0.0, atmosphere.earthRadius) + vec3(rayLength) * viewDirection);
		// Compensate for roundoff errors when the dot product is near 1
		float sunAngle = acos(0.99999 * dot(sun.direction, vertical));

		// Multiply transmittance
		vec3 layerTransmittance = sampleTable(transmittanceSampler, layer, viewRay.angle);
		scatteredLight *= layerTransmittance;

		// Add inscattering, attenuated by optical depth to the sun
		vec3 rayleighInscattering =
			atmosphere.rayleighCoefficient *
			layers.rayleighDensities[layer] *
			rayleighPhase;
		vec3 mieInscattering =
			atmosphere.mieCoefficient *
			layers.mieDensities[layer] *
			miePhase;
		vec3 transmittance = sampleTable(totalTransmittanceSampler, layer, sunAngle);
		scatteredLight += rayLength * sun.color * transmittance * (rayleighInscattering + mieInscattering);
	}
	// TODO apply these as post-processing effect to entire scene
	// Poor man's HDR
	scatteredLight = 0.3 * log(vec3(1.0) + 10.0 * scatteredLight);
	// Poor man's dithering
	scatteredLight += (fract(sin(dot(viewDirection, vec3(12.9898, 23.957, 78.233))) * 43758.5453) - 0.5) / 64.0;
}

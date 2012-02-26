#version 150

const float PI = 3.1415926535;

struct Ray {
	float height;
	float angle;
};

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

struct AtmosLayers {
	float heights[32];
	float rayleighDensities[32];
	float mieDensities[32];
};

uniform AtmosParams params;
uniform Sun sun;
uniform AtmosLayers layers;
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

void main() {
	vec3 viewDirection = normalize(viewDirectionUnnormalized);

	float lightAngle = acos(dot(viewDirection, sun.direction));
	float groundViewAngle = acos(viewDirection.z);
	float groundSunAngle = acos(sun.direction.z);
	Ray groundViewRay = Ray(params.earthRadius, groundViewAngle);
	Ray groundSunRay = Ray(params.earthRadius, groundSunAngle);

	vec3 rayleighPhase = vec3(rayleighPhaseFunction(lightAngle));
	vec3 miePhase = vec3(miePhaseFunction(lightAngle));

	scatteredLight = (1.0 - smoothstep(sun.angularRadius, sun.angularRadius * 1.2, lightAngle)) * sun.color;
	scatteredLight *= 0.5 + 0.5 * sign(viewDirection.z);
	for (int layer = params.numLayers - 2; layer >= 0; --layer) {
		float height = layers.heights[layer];

		Ray viewRay = Ray(height, rayAngleUpwards(groundViewRay, height));
		float rayLength = rayLengthUpwards(viewRay, layers.heights[layer + 1]);
		float rayLengthLong;
		if (layer == 0 && groundViewAngle > 0.5 * PI) {
			// This ray segment passes through the ground.
			// Pretend the ground is made of fog, so it'll be white (by day).
			// Without this, the ground would be a vacuum and we'd see the sky mirrored in the horizon.
			viewRay.angle = PI - viewRay.angle;
			rayLengthLong = rayLengthUpwards(viewRay, layers.heights[layer + 1]);
		} else {
			rayLengthLong = rayLength;
		}

		// TODO this is probably wrong; needs to use the total ray length from the ground to this point,
		// not the ray length of this particular segment
		vec3 vertical = normalize(vec3(0.0, 0.0, params.earthRadius) + vec3(rayLength) * viewDirection);
		// Compensate for roundoff errors when the dot product is near 1
		float sunAngle = acos(0.99999 * dot(sun.direction, vertical));

		// Multiply transmittance
		vec3 layerTransmittance = sampleTable(transmittanceSampler, layer, viewRay.angle);
		scatteredLight *= layerTransmittance;

		// Add inscattering, attenuated by optical depth to the sun
		vec3 rayleighInscattering =
			params.rayleighCoefficient *
			layers.rayleighDensities[layer] *
			rayleighPhase;
		vec3 mieInscattering =
			params.mieCoefficient *
			layers.mieDensities[layer] *
			miePhase;
		vec3 transmittance = sampleTable(totalTransmittanceSampler, layer, sunAngle);
		scatteredLight += rayLengthLong * sun.color * transmittance * (rayleighInscattering + mieInscattering);
	}
	// TODO apply these as post-processing effect to entire scene
	// Poor man's HDR
	scatteredLight = 0.3 * log(vec3(1.0) + 10.0 * scatteredLight);
	// Poor man's dithering
	scatteredLight += (fract(sin(dot(viewDirection, vec3(12.9898, 23.957, 78.233))) * 43758.5453) - 0.5) / 64.0;
}

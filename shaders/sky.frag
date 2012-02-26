#version 150

const float PI = 3.1415926535;

struct Ray {
	float height;
	float angle;
};

struct Atmosphere {
	float earthRadius;
	vec3 rayleighCoefficient;
	vec3 rayleighExtinctionCoefficient;
	vec3 mieCoefficient;
	float mieDirectionality;
	vec3 mieExtinctionCoefficient;
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
uniform sampler2DRect totalTransmittanceSampler;

in vec3 viewDirectionUnnormalized;

out vec3 scatteredLight;

float pow2(float x) {
	return x * x;
}

bool rayHitsHeight(Ray ray, float targetHeight) {
	return ray.height * sin(ray.angle) < targetHeight;
}

float rayLengthUpwards(Ray ray, float targetHeight) {
	float cosAngle = cos(ray.angle);
	return sqrt(
			pow2(ray.height) * (pow2(cosAngle) - 1.0) +
			pow2(targetHeight))
		- ray.height * cosAngle;
}

float rayLengthToSameHeight(Ray ray) {
	return -2.0 * ray.height * cos(ray.angle);
}

float rayLengthDownwards(Ray ray, float targetHeight) {
	float cosAngle = cos(ray.angle);
	return -sqrt(
			pow2(ray.height) * (pow2(cosAngle) - 1.0) +
			pow2(targetHeight))
		- ray.height * cosAngle;
}

float rayAngleUpwards(Ray ray, float targetHeight) {
	return asin(ray.height * sin(ray.angle) / targetHeight);
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

// Ray length from the given layer to the next,
// where 'next' might be the one above (for angle <= 0.5pi),
// the layer itself (for 0.5pi < angle < x),
// or the layer below it (for x <= angle).
// ray.height is assumed to be equal to layers.height[layer].
float rayLengthToNextLayer(Ray ray, int layer) {
	if (ray.angle <= 0.5 * PI && layer == layers.numLayers - 1) {
		// Ray goes up into space
		return 0.0;
	} else if (ray.angle <= 0.5 * PI) {
		// Ray goes up, hits layer above
		return rayLengthUpwards(ray, layers.heights[layer + 1]);
	} else if (layer == 0) {
		// Ray goes down, into the ground; near infinite
		return 1e30;
	} else if (!rayHitsHeight(ray, layers.heights[layer - 1])) {
		// Ray goes down, misses layer below, hits current layer from below
		return rayLengthToSameHeight(ray);
	} else {
		// Ray goes down, hits layer below
		return rayLengthDownwards(ray, layers.heights[layer - 1]);
	}
}

vec3 transmittanceToNextLayer(Ray ray, int layer) {
	float rayLength = rayLengthToNextLayer(ray, layer);
	int nextLayer = ray.angle <= 0.5 * PI ? layer : (max(1, layer) - 1);
	vec3 extinction =
		atmosphere.rayleighExtinctionCoefficient * layers.rayleighDensities[nextLayer] +
		atmosphere.mieExtinctionCoefficient * layers.mieDensities[nextLayer];
	return exp(-extinction * rayLength);
}

void main() {
	vec3 viewDirection = normalize(viewDirectionUnnormalized);

	float lightAngle = acos(dot(viewDirection, sun.direction));
	float groundViewAngle = acos(viewDirection.z);
	float groundSunAngle = acos(sun.direction.z);
	Ray groundViewRay = Ray(atmosphere.earthRadius, groundViewAngle);
	Ray groundSunRay = Ray(atmosphere.earthRadius, groundSunAngle);

	vec3 rayleighPhase = vec3(rayleighPhaseFunction(lightAngle));
	vec3 miePhase = vec3(miePhaseFunction(lightAngle));

	scatteredLight = (1.0 - smoothstep(sun.angularRadius, sun.angularRadius * 1.2, lightAngle)) * sun.color;
	scatteredLight *= 0.5 + 0.5 * sign(viewDirection.z);
	for (int layer = layers.numLayers - 2; layer >= 0; --layer) {
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

		vec3 vertical = normalize(vec3(0.0, 0.0, atmosphere.earthRadius) + vec3(rayLength) * viewDirection);
		// Compensate for roundoff errors when the dot product is near 1
		float sunAngle = acos(0.99999 * dot(sun.direction, vertical));

		// Multiply transmittance
		vec3 layerTransmittance = transmittanceToNextLayer(viewRay, layer);
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
		scatteredLight += rayLengthLong * sun.color * transmittance * (rayleighInscattering + mieInscattering);
	}
	// TODO apply these as post-processing effect to entire scene
	// Poor man's HDR
	scatteredLight = 0.3 * log(vec3(1.0) + 10.0 * scatteredLight);
	// Poor man's dithering
	scatteredLight += (fract(sin(dot(viewDirection, vec3(12.9898, 23.957, 78.233))) * 43758.5453) - 0.5) / 64.0;
}

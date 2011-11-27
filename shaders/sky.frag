#version 120

uniform samplerCube skySampler;

in vec3 direction;

void main() {
    gl_FragColor = textureCube(skySampler, direction);
}

#version 450

// in/out locations are separate. 0 != 0
layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragTex;

layout(set = 1, binding = 0) uniform sampler2D textureSampler;

layout(location = 0) out vec4 outColor; // Final output color. Must also have location.

void main() {
    outColor = texture(textureSampler, fragTex);
}
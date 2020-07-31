#version 450

layout(location = 0) in vec3 fragColor; // Interpolated color from vertex based on location.
// in/out locations are separate.
layout(location = 0) out vec4 outColor; // Final output color. Must also have location.

void main() {
    outColor = vec4(fragColor, 1.0);
}
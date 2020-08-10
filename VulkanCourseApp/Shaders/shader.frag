#version 450

// in/out locations are separate. 0 != 0
layout(location = 0) in vec3 fragCol;
layout(location = 0) out vec4 outColor; // Final output color. Must also have location.

void main() {
    outColor = vec4(fragCol, 1.0);
}
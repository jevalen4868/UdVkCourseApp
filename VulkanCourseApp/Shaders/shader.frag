#version 450

// in/out locations are separate. 0 != 0
layout(location = 0) out vec4 outColor; // Final output color. Must also have location.

void main() {
    outColor = vec4(1.0, 0.0, 0.0, 1.0);
}
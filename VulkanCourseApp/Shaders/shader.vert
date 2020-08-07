#version 450 // Use GLSL Version 4.5.0

layout(blocation = 0) in vec3 pos;

void main() {
    gl_Position = vec4(pos, 1.0);    
}
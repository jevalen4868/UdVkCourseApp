#version 450 // Use GLSL Version 4.5.0

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;

layout(set = 0, binding = 0) uniform UboViewProjection {
    mat4 proj;
    mat4 view;    
} uboViewProjection;

layout(set = 0, binding = 1) uniform UboModel {
    mat4 model;    
} uboModel;

layout(location = 0) out vec3 fragCol;

void main() {
    // Matrix multiplication goes right to left.
    gl_Position = uboViewProjection.proj * uboViewProjection.view * uboModel.model * vec4(pos, 1.0);  
    fragCol = col;  
}
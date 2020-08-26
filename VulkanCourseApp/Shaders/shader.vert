#version 450 // Use GLSL Version 4.5.0

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
layout(location = 2) in vec2 tex;

layout(set = 0, binding = 0) uniform UboViewProjection {
    mat4 proj;
    mat4 view;    
} uboViewProjection;

// NOT IN USE, LEFT FOR REFERENCE.
layout(set = 0, binding = 1) uniform UboModel {
    mat4 model;    
} uboModel;

// Only one push constant block available.
layout(push_constant) uniform PushModel {
    mat4 model;
} pushModel;

layout(location = 0) out vec3 fragCol;
layout(location = 1) out vec2 fragTex;

void main() {
    // Matrix multiplication goes right to left.
    gl_Position = uboViewProjection.proj * uboViewProjection.view * pushModel.model * vec4(pos, 1.0);  
    fragCol = col; 
    fragTex = tex; 
}
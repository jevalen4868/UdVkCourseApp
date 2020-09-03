#version 450

// Coming from input attachment index 0. 
layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputColor; //Color input from subpass1.
layout(input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth; //Depth input from subpass1.

layout(location = 0) out vec4 color;

void main() {
    int xHalf = 1920 / 2;
    if(gl_FragCoord.x > xHalf) {
        float lowerBound = 0.98;
        float upperBound = 1.00;

        float depth = subpassLoad(inputDepth).r;
        float depthColorScaled = 1.0f - ((depth - lowerBound) / (upperBound - lowerBound));
        color = vec4(subpassLoad(inputColor).rgb * depthColorScaled, 1.0f);
    } else {
        color = subpassLoad(inputColor).rgba;
    }
}
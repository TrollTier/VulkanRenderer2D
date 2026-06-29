#version 450

struct Circle {
    vec4 color;
    vec4 position;
    float radius;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(std430, set = 1, binding = 0) readonly buffer CircleBuffer {
    Circle circles[];
} circleBuffer;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 texCoord;
layout(location = 2) out vec3 outPosition;
layout(location = 3) out float radius;

void main() {
    Circle circle = circleBuffer.circles[gl_InstanceIndex];

    gl_Position = vec4(inPosition, 1.0);

    fragColor = circle.color.xyz;
    texCoord = vec3(inTexCoord, 0);
    outPosition = circle.position.xyz;
    radius = circle.radius;
}
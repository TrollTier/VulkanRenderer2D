#version 450

struct Circle {
    vec3 color;
    vec2 position;
    float radius;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(std430, set = 1, binding = 0) readonly buffer CircleBuffer {
    Circle circles[];
} circleBuffer;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 texCoord;
layout(location = 2) out vec2 outPosition;
layout(location = 3) out float radius;

void main() {
    Circle circle = circleBuffer.circles[gl_InstanceIndex];

    vec4 position = vec4(circle.position, 1.0, 1.0) * vec4(inPosition, 1.0);
    gl_Position = position;

    fragColor = circle.color;
    texCoord = inTexCoord.xy;
    outPosition = circle.position;
    radius = circle.radius;
}
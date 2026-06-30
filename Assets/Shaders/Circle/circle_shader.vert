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
layout(location = 1) out vec2 localCoordinates;
layout(location = 2) out float radius;

float remapToNDC(float x) {
    return x * 2.0 - 1.0;
}

void main() {
    Circle circle = circleBuffer.circles[gl_InstanceIndex];

    vec2 localPosition = (inPosition.xy * 2.0 - 1.0) * circle.radius;
    vec2 worldPosition = circle.position.xy + localPosition;

    gl_Position = vec4(
        remapToNDC(worldPosition.x),
        remapToNDC(worldPosition.y),
        0.0,
        1.0);

    fragColor = circle.color.xyz;
    localCoordinates = localPosition;
    radius = circle.radius;
}
#version 450

struct Rectangle {
    vec4 color;
    vec4 center;
    float scaleX;
    float scaleY;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(std430, set = 1, binding = 0) readonly buffer RectangleBuffer {
    Rectangle rectangles[];
} rectangleBuffer;

layout(std430, set = 2, binding = 0) readonly buffer InstanceIndexBuffer {
    uint indices[];
} instanceIndexBuffer;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 localCoordinates;

float remapToNDC(float x) {
    return x * 2.0 - 1.0;
}

void main() {
    uint realIndex = instanceIndexBuffer.indices[gl_InstanceIndex];
    Rectangle rectangle = rectangleBuffer.rectangles[realIndex];

    vec2 localPosition = vec2(
        (inPosition.x * 2.0 - 1.0) * rectangle.scaleX,
        (inPosition.y * 2.0 - 1.0) * rectangle.scaleY);

    vec2 worldPosition = rectangle.center.xy + localPosition;

    gl_Position = vec4(
        remapToNDC(worldPosition.x),
        remapToNDC(worldPosition.y),
        0.0,
        1.0);

    fragColor = rectangle.color.xyz;
    localCoordinates = localPosition;
}
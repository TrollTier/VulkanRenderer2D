#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec2 position;
layout(location = 3) in float radius;

layout(location = 0) out vec4 outColor;

void main() {
    if (distance(texCoord, position) <= radius)
    {
        outColor = vec4(fragColor, 1.0f);
    }
    else
    {
        outColor = vec4(0, 0, 0, 0);
    }
}
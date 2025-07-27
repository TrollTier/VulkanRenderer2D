#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform sampler2D textures[];

layout(push_constant) uniform ModelData {
    mat4 modelMatrix;
    uint textureIndex;
} ubo;

void main() {
    outColor = texture(textures[nonuniformEXT(ubo.textureIndex)], fragTexCoord);
}
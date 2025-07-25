#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 modelMatrix;
} ubo;

layout( push_constant ) uniform constants
{
	mat4 viewProjection;
} Camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;


void main() {
    vec4 position = Camera.viewProjection * ubo.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = position;
    fragColor = vec3(position.x, position.y, position.z);
    fragTexCoord = inTexCoord;
}
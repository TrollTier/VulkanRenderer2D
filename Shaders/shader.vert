#version 450

struct ObjectData{
	mat4 modelMatrix;
	uint textureIndex;
};

layout(binding = 0) uniform CameraUniformData {
    mat4 viewProjection;
} camera;

layout(std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
} objectBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) flat out uint textureIndex;

void main() {
    vec4 position = camera.viewProjection * objectBuffer.objects[gl_InstanceIndex].modelMatrix * vec4(inPosition, 1.0);
    gl_Position = position;
    fragColor = vec3(position.x, position.y, position.z);
    fragTexCoord = inTexCoord;
    textureIndex = objectBuffer.objects[gl_InstanceIndex].textureIndex;
}
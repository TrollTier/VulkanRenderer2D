#version 450

struct ImageRect {
    float translateX;
    float translateY;
    float scaleX;
    float scaleY;
};

struct ObjectData{
	mat4 modelMatrix;
	ImageRect spriteFrame;
	uint textureIndex;
};

layout(binding = 0) uniform CameraUniformData {
    mat4 viewProjection;
} camera;

layout(std430, set = 1, binding = 0) readonly buffer ObjectBuffer {
    ObjectData objects[];
} objectBuffer;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) flat out uint textureIndex;

void main() {
    ObjectData instanceData = objectBuffer.objects[gl_InstanceIndex];

    vec4 position = camera.viewProjection * instanceData.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = position;

    fragColor = vec3(position.x, position.y, position.z);
    textureIndex = instanceData.textureIndex;

    mat3 uvTransform = mat3(
        vec3(instanceData.spriteFrame.scaleX, 0, 0),
        vec3(0, instanceData.spriteFrame.scaleY, 0),
        vec3(instanceData.spriteFrame.translateX, instanceData.spriteFrame.translateY, 1.0)
    );

    fragTexCoord = (uvTransform * vec3(inTexCoord, 1.0)).xy;
}
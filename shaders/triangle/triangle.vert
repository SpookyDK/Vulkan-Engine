#version 450

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} camera;

// Set = 1, binding = 0
layout(set = 1, binding = 0) readonly buffer ObjectDataBuffer {
    mat4 models[];
} objectBuffer

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uint inObjectIndex;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

const vec3 instanceOffsets[4] = vec3[4](
    vec3(-0.5, -0.5, 0.0),
    vec3( 0.5, -0.5, 0.0),
    vec3(-0.5,  0.5, 0.0),
    vec3( 0.5,  0.5, 0.0)
);

void main(){
    vec3 pos = inPosition + instanceOffsets[gl_InstanceIndex];
    gl_Position = camera.proj * camera.view * object.model * vec4(pos,1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}

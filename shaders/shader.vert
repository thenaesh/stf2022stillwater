#version 450

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform constants
{
	vec4 time;
    mat4 rotationMatrix;
} PushConstants;


void main() {
    gl_Position = PushConstants.rotationMatrix * vec4(vertexPosition, 1.0);
    fragColor = vertexColor;
}

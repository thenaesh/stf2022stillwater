#version 450

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in ivec2 vertexGridCoords;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform constants
{
    vec2 time;
    ivec2 gridBounds;
    mat4 colorChangeMatrix;
    mat4 cameraMatrix;
    mat4 perspectiveMatrix;
} PushConstants;


float t = PushConstants.time.x;
float theta = PushConstants.time.y;

float amplitude(float r0, float c0, float r, float c, float rMax, float cMax) {
    return exp(-(pow((r - r0)/(rMax - r0), 2) + pow((c - c0)/(cMax - c0), 2)));
}

float r0 = PushConstants.gridBounds.x / 2;
float c0 = PushConstants.gridBounds.y / 2;
float r = vertexGridCoords.x;
float c = vertexGridCoords.y;
float rMax = PushConstants.gridBounds.x;
float cMax = PushConstants.gridBounds.y;

float A = amplitude(r0, c0, r, c, rMax, cMax) * 0.3;

vec3 transformZ(vec3 src) {
    return vec3(src.x, src.y, A * sin(theta));
}

void main() {
    gl_Position = PushConstants.perspectiveMatrix * PushConstants.cameraMatrix * vec4(transformZ(vertexPosition), 1.0);
    fragColor = (PushConstants.colorChangeMatrix * vec4(vertexColor, 1.0)).xyz;
    // fragColor = vertexColor;
}

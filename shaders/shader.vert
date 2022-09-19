#version 450

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in ivec2 vertexGridCoords;

layout(location = 0) out vec3 fragColor;

layout(push_constant) uniform constants
{
    vec2 time;
    ivec2 gridBounds;
    uvec2 jaga;
    uvec2 agaj;
    uvec2 gaja;
    uvec2 ajag;
    mat4 viewTransformMatrix;
} PushConstants;


float pi = 3.141592654;
float t = PushConstants.time.x;

uvec2 jaga = PushConstants.jaga;
uvec2 agaj = PushConstants.agaj;
uvec2 gaja = PushConstants.gaja;
uvec2 ajag = PushConstants.ajag;

mat3 A;
mat3 B;
mat3 C;
mat3 D;

float r = vertexGridCoords.x;
float c = vertexGridCoords.y;
float rMax = PushConstants.gridBounds.x;
float cMax = PushConstants.gridBounds.y;

float amp(float r0, float c0, float r, float c, float rMax, float cMax) {
    return exp(-(pow((r - r0)/(rMax - r0), 2) + pow((c - c0)/(cMax - c0), 2))) * 0.1;
}

vec3 transformZ(vec3 src, mat3 V, mat3 W) {
    float z = 0.0;
    float N = 30.0;
    for (float i = N/2; i < N; i++) {
        float dz;

        if (mod(i, 2) == 0) {
            dz = (i/N) * amp(i*rMax/N, i*cMax/N, r, c, rMax, cMax) * sin(2 * pi * i * t / N);
        } else {
            dz = (i/N) * amp(i*rMax/N, i*cMax/N, r, c, rMax, cMax) * cos(2 * pi * i * t / N);
        }

        int c3 = int(mod(c, 3));
        int r3 = int(mod(r, 3));
        dz *= clamp(abs(W[c3][r3] - V[c3][r3]), 0.0, 1.0);

        z += dz;
    }
    return vec3(src.x, src.y, z);
}


mat4 viewTransform = PushConstants.viewTransformMatrix;

void main() {
    A[0] = vec3(float(jaga.x), float(jaga.y), float(agaj.x));
    A[1] = vec3(float(agaj.y), float(gaja.x), float(gaja.x));
    A[2] = vec3(float(gaja.y), float(ajag.x), float(ajag.y));

    B[0] = vec3(1.0, 0.0, 0.0);
    B[1] = vec3(0.0, 1.0, 0.0);
    B[2] = vec3(0.0, 0.0, 1.0);

    C = A * B;

/*
jaga 1345532779, 1077705517    uint2
agaj 1412775461, 1819044159    uint2
gaja 1044332604, 758933588    uint2
ajag 857818482, 759053357    uint2
 */

    D[0] = vec3(1345532779.0, 1077705517.0, 1412775461.0);
    D[1] = vec3(1819044159.0, 1044332604.0, 1044332604.0);
    D[2] = vec3(758933588.0, 857818482.0, 759053357.0);

    // NOTE: z is the upwards direction
    gl_Position = viewTransform * vec4(transformZ(vertexPosition, C, D), 1.0);
    fragColor = vertexColor;
    // fragColor = vec3(0.02, 0.48, 0.52);
}

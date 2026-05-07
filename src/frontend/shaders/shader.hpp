//Vertex Buffer Object - chunk of memory on the gpu;
//upload vertex data (coords) lives until manually deleted
inline const char* default_vert = R"(
#version 330 core

layout(location = 0) in vec2 aPos;      //comes from VAO
out vec2 vUV;      // passed to frag shader

void main() {
    vUV = aPos * 0.5 + 0.5;  // remap -1..1 to 0..1
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";


// Vertex Array Object - how to interpret the VBO; 
// how many bytes per vertex, which bytes are pos, 
// which are uv etc.. a schema for the VBO
inline const char* default_frag = R"(
#version 330 core

in vec2 vUV;       // received from vert shader
out vec4 fragColor;

uniform vec2 iResolution;
uniform float iTime;

float rand(vec2 n) {
    return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

void main() {
    float aspect = iResolution.x / iResolution.y;
    vec2 cell = floor(vUV * 10.0); // which cell (10x10 grid)
    vec2 local = fract(vUV * 10.0); // position within cell

    // vec2 uv = vec2(vUV.x * aspect, vUV.y);
    // vec2 center = vec2(0.5);// * aspect + sin(iTime) * 0.3, 0.5 + cos(iTime) * 0.3);
    vec2 center = vec2(
        fract(rand(cell) + iTime * 0.1),
        fract(rand(cell + 1.0) + iTime * 0.07)
    );
    float d = distance(local, center);
    float circle = smoothstep(0.02, 0.01, d);
    fragColor = vec4(vec3(0.91, 0.44, 0.29) * circle + vec3(0.145, 0.145, 0.141), 1.0);
}
)";

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

void main() {
    fragColor = vec4(vUV, 0.0, 1.0);
}
)";

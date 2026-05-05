#version 330 core

//Vertex Buffer Object - chunk of memory on the gpu;
//upload vertex data (coords) lives until manually deleted

in vec2 aPos;      //comes from VAO
out vec2 vUV;      // passed to frag shader

void main() {
    vUV = aPos * 0.5 + 0.5;  // remap -1..1 to 0..1
    gl_Position = vec4(aPos, 0.0, 1.0);
}

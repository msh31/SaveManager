#version 330 core

// Vertex Array Object - how to interpret the VBO; 
// how many bytes per vertex, which bytes are pos, 
// which are uv etc.. a schema for the VBO

in vec2 vUV;       // received from vert shader
out vec4 fragColor;

void main() {
    fragColor = vec4(vUV, 0.0, 1.0);
}

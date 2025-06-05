#version 330 core
uniform vec3 lineColor; // e.g. (1,0,0)
out vec4 FragColor;

void main() {
    FragColor = vec4(lineColor, 1.0);
}

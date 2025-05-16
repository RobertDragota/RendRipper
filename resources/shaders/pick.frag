#version 330 core
uniform vec3 uPickColor; // RGB encoded ID
out vec4 FragColor;

void main() {
    FragColor = vec4(uPickColor, 1.0);
}

#version 330 core
in  vec2 vTexCoords;
out vec4 FragColor;

uniform sampler2D plateTexture;

void main() {
    vec4 tex = texture(plateTexture, vTexCoords);
    FragColor = tex;
}
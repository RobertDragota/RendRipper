#version 330 core
layout(location = 0) in vec3 aPos;        // vertex position
layout(location = 1) in vec2 aTexCoords;  // UVs

out vec2 vTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vTexCoords   = aTexCoords;
    gl_Position  = projection * view * model * vec4(aPos, 1.0);
}
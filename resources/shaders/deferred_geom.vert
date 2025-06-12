#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 objectColor = vec4(0.7,0.7,0.7,1.0);

out vec3 FragPos;
out vec3 Normal;
out vec4 Albedo;

void main(){
    vec4 world = model * vec4(aPos,1.0);
    FragPos = world.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    Albedo = objectColor;
    gl_Position = projection * view * world;
}

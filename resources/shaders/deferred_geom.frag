#version 330 core
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gAlbedo;

in vec3 FragPos;
in vec3 Normal;
in vec4 Albedo;

void main(){
    gPosition = FragPos;
    gNormal = normalize(Normal);
    gAlbedo = Albedo;
}

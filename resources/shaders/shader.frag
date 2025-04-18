#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D texture_diffuse1;
void main() {
    vec3 color = texture(texture_diffuse1, TexCoords).rgb;
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 ambient = 0.1 * color;
    FragColor = vec4(ambient + diff * color, 1.0);
}
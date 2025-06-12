#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform vec3 lightDir;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main(){
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal  = texture(gNormal, TexCoords).rgb;
    vec4 Albedo  = texture(gAlbedo, TexCoords);
    vec3 N = normalize(Normal);
    vec3 Ld = normalize(-lightDir);
    vec3 Lp = normalize(lightPos - FragPos);
    vec3 L  = normalize(Ld + Lp);
    vec3 ambient = 0.2 * Albedo.rgb;
    float diff = max(dot(N,L),0.0);
    vec3 diffuse = diff * Albedo.rgb;
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V,R),0.0),32.0);
    vec3 specular = 0.5 * spec * vec3(1.0);
    vec3 color = ambient + diffuse + specular;
    FragColor = vec4(color, Albedo.a);
}

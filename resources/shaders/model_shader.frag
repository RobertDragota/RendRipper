#version 330 core

in  vec3 FragPos;
in  vec3 Normal;
out vec4 FragColor;

uniform vec3 lightDir;      // Directional light
uniform vec3 lightPos;      // Light world position (optional)
uniform vec3 viewPos;       // Camera world position
uniform vec4 objectColor;   // e.g. vec4(1,0.5,0,1)

void main() {
    // **Ambient**
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * objectColor.rgb;

    // **Diffuse** (Lambert)
    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * objectColor.rgb;

    // **Specular**
    float specStrength = 0.5;
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), 32);
    vec3 specular = specStrength * spec * vec3(1.0);

    vec3 color = ambient + diffuse + specular;
    FragColor  = vec4(color, objectColor.a);
}

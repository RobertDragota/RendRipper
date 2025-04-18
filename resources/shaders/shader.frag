#version 330 core
in vec3 fPos;
in vec3 fNorm;
in vec2 fTex;
out vec4 FragColor;
uniform sampler2D texture_diffuse1;
uniform vec4 slicingPlane;
void main(){
    if(dot(vec4(fPos,1),slicingPlane)>0) discard;
    vec3 col=texture(texture_diffuse1,fTex).rgb;
    float d=max(dot(normalize(fNorm),normalize(vec3(0.5,1,0.3))),0);
    FragColor=vec4(d*col,1);
}
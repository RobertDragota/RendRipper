#version 330 core

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

in  vec3 vsWorldPos[];   // from vertex shader
out vec3 gsWorldPos;     // to fragment shader
out vec3 gsNormal;       // to fragment shader: face normal
out vec4 gsColor;        // to fragment shader: RGBA color

uniform mat4 view;
uniform mat4 projection;
uniform float lineWidth; // thickness (in mm)
uniform vec4  lineColor; // RGBA for this category

// Extract camera’s right vector from view
vec3 GetCameraRight()
{
    mat4 invV = inverse(view);
    return normalize(vec3(invV[0][0], invV[1][0], invV[2][0]));
}
// Extract camera’s up vector from view
vec3 GetCameraUp()
{
    mat4 invV = inverse(view);
    return normalize(vec3(invV[0][1], invV[1][1], invV[2][1]));
}

void main()
{
    // 1) Two endpoints in world space
    vec3 P0 = vsWorldPos[0];
    vec3 P1 = vsWorldPos[1];
    vec3 dir = normalize(P1 - P0);

    // 2) Build a “side” vector by crossing dir with camera up/right
    vec3 camUp    = GetCameraUp();
    vec3 camRight = GetCameraRight();
    vec3 perpA    = cross(dir, camUp);
    vec3 perpB    = cross(camRight, dir);
    vec3 side     = (length(perpA) > length(perpB)) ? normalize(perpA) : normalize(perpB);

    // 3) Scale by half the thickness (in mm)
    vec3 offset = side * (lineWidth * 0.5);

    // 4) Build four corners of the quad
    vec3 A = P0 + offset;
    vec3 B = P1 + offset;
    vec3 C = P0 - offset;
    vec3 D = P1 - offset;

    // 5) Compute face normal for shading
    vec3 edge1 = B - A;
    vec3 edge2 = C - A;
    vec3 faceNormal = normalize(cross(edge1, edge2));

    // 6) Emit A
    {
        vec4 clipA = projection * view * vec4(A, 1.0);
        gl_Position = clipA;
        gsWorldPos = A;
        gsNormal   = faceNormal;
        gsColor    = lineColor;
        EmitVertex();
    }
    // 7) Emit B
    {
        vec4 clipB = projection * view * vec4(B, 1.0);
        gl_Position = clipB;
        gsWorldPos = B;
        gsNormal   = faceNormal;
        gsColor    = lineColor;
        EmitVertex();
    }
    // 8) Emit C
    {
        vec4 clipC = projection * view * vec4(C, 1.0);
        gl_Position = clipC;
        gsWorldPos = C;
        gsNormal   = faceNormal;
        gsColor    = lineColor;
        EmitVertex();
    }
    // 9) Emit D
    {
        vec4 clipD = projection * view * vec4(D, 1.0);
        gl_Position = clipD;
        gsWorldPos = D;
        gsNormal   = faceNormal;
        gsColor    = lineColor;
        EmitVertex();
    }

    EndPrimitive();
}

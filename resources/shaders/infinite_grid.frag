#version 330 core
in vec3 vWorldPos;
out vec4 FragColor;

uniform float gridSpacing = 1.0;
uniform float lineWidthFactor = 0.03;

// Make colors uniform for easier adjustment from C++ if desired later
uniform vec3 u_baseColor = vec3(0.10, 0.105, 0.11); // Match FBO clear or slightly darker
uniform vec3 u_lineColor = vec3(0.3, 0.3, 0.32);

void main() {
    vec2 coord = vWorldPos.xy / gridSpacing;
    vec2 fw = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / fw;
    float line = min(grid.x, grid.y);
    float lineAlpha = 1.0 - smoothstep(0.5 - lineWidthFactor, 0.5 + lineWidthFactor, line);

    // For an opaque grid plane:
    vec3 finalColor = mix(u_baseColor, u_lineColor, lineAlpha);
    FragColor = vec4(finalColor, 1.0);

    // For semi-transparent grid lines (requires blending enabled in SceneRenderer::RenderGridAndVolume)
    // FragColor = vec4(u_lineColor, lineAlpha);
}
#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    float time = fragColor.x;
    vec2 mouse = fragColor.yz;
    vec2 uv = fragTexCoord;
    float dist = distance(uv, mouse);
    float glow = exp(-dist * 9.0) * (0.65 + 0.35 * sin(time * 3.0));
    float ring = exp(-pow(dist - 0.18, 2.0) * 900.0) * (0.5 + 0.5 * sin(time * 2.0));
    vec3 col = vec3(0.02, 0.03, 0.06) + glow * vec3(0.25, 0.45, 1.0) + ring * vec3(0.15, 0.3, 0.8);
    outColor = vec4(col, 1.0);
}

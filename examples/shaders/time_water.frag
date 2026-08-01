#version 450
layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
    float time = fragColor.x;
    vec2 uv = fragTexCoord;
    float wave = sin(uv.x * 14.0 + time * 2.5) * sin(uv.y * 11.0 - time * 1.9);
    float ripple = sin(distance(uv, vec2(0.5)) * 22.0 - time * 3.0) * 0.5;
    vec3 col = mix(vec3(0.03, 0.07, 0.2), vec3(0.1, 0.45, 0.75), 0.5 + 0.5 * wave);
    col += vec3(0.2, 0.5, 0.9) * (ripple * 0.25);
    outColor = vec4(col, 1.0);
}

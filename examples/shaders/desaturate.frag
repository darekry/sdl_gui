#version 450
layout(set = 2, binding = 0) uniform sampler2D uTexture;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor;

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
    float hue = fragTexCoord.x;
    vec3 rainbow = hsv2rgb(vec3(hue, 0.8, 0.9));
    outColor = vec4(rainbow, 1.0);
}

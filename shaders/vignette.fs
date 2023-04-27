#version 400

uniform float Time;
uniform vec2 Resolution;

void main() {
    vec2 pos = (gl_FragCoord.xy / Resolution.xy) * 2.0 - 1.0;
    float distFromCenter = length(pos);
    float alpha = 1.0;
    float intensity = 1.0;
    if (Time > 6.0) {
        alpha = smoothstep(0.0, 1.0, distFromCenter);
        if (Time < 7.0) {
            alpha = mix(1.0, alpha, (Time - 6.0) / 1.0f);
        }
        if (Time < 8.0) {
            intensity = smoothstep(1.0, 0.0, (Time - 6.0) / 2.0f);
        }
    }
    vec3 color = alpha * intensity * vec3(1.0);
    gl_FragColor = vec4(color, 1.0);
}
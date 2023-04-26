#version 400

uniform float Time;
uniform vec2 Resolution;

void main() {
    vec2 pos = (gl_FragCoord.xy / Resolution.xy) * 2.0 - 1.0;
    float distFromCenter = length(pos);
    float alpha = 1.0;
    float intensity = 1.0;
    float startTime = 12.0;
    if (Time > startTime) {
        alpha = smoothstep(0.0, 1.0, distFromCenter);
        if (Time < startTime + 1) {
            alpha = mix(1.0, alpha, (Time - startTime) / 1.0f);
        }
        if (Time < startTime + 2) {
            intensity = smoothstep(1.0, 0.0, (Time - startTime) / 2.0f);
        }
    }
    vec3 color = alpha * intensity * vec3(1.0);
    gl_FragColor = vec4(color, 1.0);
}
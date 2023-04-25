#version 400

uniform float Time;
uniform vec2 Resolution;

void main() {
    vec2 pos = (gl_FragCoord.xy / Resolution.xy) * 2.0 - 1.0;
    float distFromCenter = length(pos);
    float alpha = smoothstep(0.0, 1.0, distFromCenter);
    float intensity = 1.0;
    if (Time > 6.0 && Time < 10.0) {
        intensity = smoothstep(1.0, 0.0, (Time - 6.0) / 4.0f);
    }
    vec3 color = alpha * intensity * vec3(1.0);
    gl_FragColor = vec4(color, 1.0);
}
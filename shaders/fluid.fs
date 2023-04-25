#version 400

uniform float Time;
uniform vec2 Resolution;
uniform vec3 BallPos;

out vec4 fragColor;

void main() {
    vec2 pixelUV = gl_FragCoord.xy / Resolution.xy;
    vec2 ballUV = BallPos.xy / Resolution.xy;
    vec3 color = vec3(0);
    int numCols = 25;
    int numRows = 15;
    for (int i = 1; i < numCols; i += 1) {
        for (int j = 1; j < numRows; j += 1) {
            vec2 quadUV = vec2(float(i) / numCols, float(j) / numRows);
            vec2 line = ballUV - quadUV;
            float p = dot(pixelUV - quadUV, line) / dot(line, line);
            vec2 pixelProj = mix(quadUV, ballUV, clamp(p, -0.5, 1.0));
            float distFromLine = (p < 0.15)? distance(pixelUV, pixelProj) : 0.5;
            color = mix(vec3(0, clamp(length(line) + sin(Time), 0.2, 0.4), 0), color, min(distFromLine / 0.05, 1));
        }
    }
    fragColor = vec4(color, 1.0);
}
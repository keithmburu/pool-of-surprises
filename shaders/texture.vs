#version 400

layout (location = 0) in vec3 vPositions;
layout (location = 1) in vec3 vNormals;
layout (location = 2) in vec2 vUVs;

uniform mat4 MVP;
uniform mat4 ModelMatrix;

out vec2 uv;

void main()
{
  uv = vUVs;
  gl_Position = MVP * vec4(vPositions, 1.0);
}

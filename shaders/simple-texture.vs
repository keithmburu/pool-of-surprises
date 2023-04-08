#version 400

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vUV;

uniform mat4 MVP;
out vec2 uv;

void main()
{
  uv = vUV;
  gl_Position = MVP * vec4(vPosition, 1.0); 
}

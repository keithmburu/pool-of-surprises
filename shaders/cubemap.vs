#version 400

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoords;

uniform mat4 MVP;
uniform mat4 ModelMatrix;
uniform bool PoolBall;
uniform bool EyeOfSauron;

out vec2 uv;
out vec3 fPos;
out vec3 fNormal;
out vec3 vPos;

void main()
{
   fPos = vec3(ModelMatrix * vec4(vPosition, 1.0));
   fNormal = vec3(ModelMatrix * vec4(vNormal, 0.0));
   gl_Position = MVP * vec4(vPosition, 1.0);
   vPos = vec3(MVP * vec4(vPosition, 1.0));
   if (PoolBall) {
      if (vTextureCoords.x > 0.5) {
         uv = vec2(2 * vTextureCoords.x, vTextureCoords.y);
      } else {
         uv = vec2(1, 0.5);
      }
   } else if (EyeOfSauron) {
      if (vTextureCoords.x > 0.5) {
         uv = vec2(2 * vTextureCoords.x, -vTextureCoords.y);
      } else {
         uv = vec2(0.5 * vTextureCoords.x, 0);
      }
   } else {
      uv = vec2(vTextureCoords.x, -vTextureCoords.y);
   }
}

#version 400

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTextureCoords;

uniform mat3 NormalMatrix;
uniform mat4 ModelViewMatrix;
uniform mat4 MVP;
uniform bool HasUV;

uniform mat4 ViewMatrix;
uniform mat4 ProjMatrix;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec4 materialColor; 

uniform vec3 eyePos;
out vec3 normalVec;
out vec3 eyeVec;
out vec3 lightVec;
out vec4 mColor;

void main()
{
   vec4 pos = ModelViewMatrix * vec4(vPos, 1.0);
   vec4 eyePos = ModelViewMatrix * vec4(eyePos, 1.0);
   vec4 lightPos = ViewMatrix * vec4(lightPos, 1.0);
   normalVec = NormalMatrix * vNormal;

   eyeVec = normalize(eyePos.xyz - pos.xyz);
   lightVec = normalize(lightPos.xyz - pos.xyz);
   gl_Position = ProjMatrix * pos;
   mColor = materialColor;
}

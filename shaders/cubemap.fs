#version 400

in vec2 uv;
in vec3 fPos;
in vec3 fNormal;
in vec3 vPos;

uniform samplerCube Cubemap;
uniform sampler2D Image;
uniform vec4 MaterialColor; 
uniform mat4 ViewMatrix;
uniform bool Skybox;
uniform vec3 CamPos;
uniform bool PoolBall;

out vec4 FragColor;

void main()
{
	vec3 I = normalize(fPos - CamPos);
   	vec3 ReflectDir = reflect(I, normalize(fNormal));
	vec4 cubemapColor = texture(Cubemap, ReflectDir);
	float relectionFactor = PoolBall? 0.5 : 0.2;
	if (Skybox) FragColor = texture(Cubemap, fPos);
	else FragColor = mix(MaterialColor * texture(Image, uv), cubemapColor, relectionFactor);
}


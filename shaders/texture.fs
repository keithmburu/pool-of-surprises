#version 400

in vec2 uv;
flat in int useTextures;

uniform sampler2D image;

out vec4 FragColor;

void main()
{
  FragColor = texture(image, uv * vec2(1, -1));
}

#version 400

in vec2 uv;

uniform sampler2D image;
uniform vec4 Color;

out vec4 FragColor;

void main()
{
  FragColor = Color * texture(image, uv * vec2(1, -1));
}

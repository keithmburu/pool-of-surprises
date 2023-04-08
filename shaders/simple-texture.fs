#version 400

uniform sampler2D Image;

in vec2 uv;
out vec4 FragColor;

void main()
{
  vec4 c = texture(Image, uv);
  FragColor = c;
}


#version 150

uniform sampler2D image;

in vec2 texCoord;

out vec4 fragment;

void main()
{
  fragment = texture(image, texCoord);
}


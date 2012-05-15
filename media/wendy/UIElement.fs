
#version 150

uniform sampler2DRect image;

in vec2 mapping;

out vec4 fragment;

void main()
{
  fragment = texture(image, mapping);
}


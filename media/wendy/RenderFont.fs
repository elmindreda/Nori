
#version 150

uniform sampler2D glyphs;
uniform vec4 color;

in vec2 texCoord;

out vec4 fragment;

void main()
{
  fragment = vec4(color.rgb, color.a * texture(glyphs, texCoord).r);
}


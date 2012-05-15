
#version 150

uniform vec2 elementPos;
uniform vec2 elementSize;
uniform vec2 texPos;
uniform vec2 texSize;

in vec2 sizeScale;
in vec2 offsetScale;
in vec2 texScale;

out vec2 mapping;

void main()
{
  vec2 position = elementPos +
                  elementSize * sizeScale +
                  texSize * offsetScale;

  mapping = texPos + texSize * texScale;

  gl_Position = wyMVP * vec4(position, 0, 1);
}


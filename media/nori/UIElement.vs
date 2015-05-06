
#version 150

uniform vec2 elementPos;
uniform vec2 elementSize;
uniform vec2 texPos;
uniform vec2 texSize;

in vec2 vSizeScale;
in vec2 vOffsetScale;
in vec2 vTexScale;

out vec2 texCoord;

void main()
{
  vec2 position = elementPos +
                  elementSize * vSizeScale +
                  texSize * vOffsetScale;

  texCoord = texPos + texSize * vTexScale;

  gl_Position = wyMVP * vec4(position, 0.0, 1.0);
}


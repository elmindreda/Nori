
uniform vec2 elementPos;
uniform vec2 elementSize;
uniform vec2 texPos;
uniform vec2 texSize;

attribute vec2 sizeScale;
attribute vec2 offsetScale;
attribute vec2 texScale;

varying vec2 mapping;

void main()
{
  vec2 position = elementPos +
                  elementSize * sizeScale +
                  texSize * offsetScale;

  mapping = texPos + texSize * texScale;

  gl_Position = wyMVP * vec4(position, 0, 1);
}


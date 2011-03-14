
uniform mat4 P;

in vec2 position;
in vec2 mapping;
in vec2 clipOverF;

varying vec2 mapping;
varying vec2 clipOverF;

void main()
{
  result.mapping = mapping;
  result.clipOverF = clipOverF;

  gl_Position = P * vec4(position, 0, 1);
}


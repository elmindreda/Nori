
#version 150

in vec2 wyPosition;

void main()
{
  gl_Position = wyP * vec4(wyPosition, 0.0, 1.0);
}


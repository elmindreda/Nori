
uniform mat4 wyP;

attribute vec2 wyPosition;

void main()
{
  gl_Position = wyP * vec4(wyPosition, 0, 1);
}


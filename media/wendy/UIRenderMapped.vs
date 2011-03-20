
uniform mat4 wyP;

attribute vec2 wyPosition;
attribute vec2 wyTexCoord;

varying vec2 texCoord;

void main()
{
  texCoord = wyTexCoord;

  gl_Position = wyP * vec4(wyPosition, 0, 1);
}


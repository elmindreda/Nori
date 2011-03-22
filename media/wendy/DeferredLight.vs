
attribute vec2 wyPosition;
attribute vec2 wyTexCoord;
attribute vec2 wyClipOverF;

varying vec2 texCoord;
varying vec2 clipOverF;

void main()
{
  texCoord = wyTexCoord;
  clipOverF = wyClipOverF;

  gl_Position = wyP * vec4(wyPosition, 0, 1);
}


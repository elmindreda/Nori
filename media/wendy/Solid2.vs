
attribute vec2 wyPosition;

void main()
{
  gl_Position = wyMVP * vec4(wyPosition, 0, 1);
}


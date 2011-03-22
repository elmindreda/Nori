
attribute vec3 wyPosition;

void main()
{
  gl_Position = wyMVP * vec4(wyPosition, 1);
}


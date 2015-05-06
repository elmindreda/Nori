
#version 150

in vec2 vPosition;

void main()
{
  gl_Position = wyP * vec4(vPosition, 0.0, 1.0);
}



#version 150

in vec2 wyPosition;
in vec2 wyTexCoord;

out vec2 texCoord;

void main()
{
  texCoord = wyTexCoord;

  gl_Position = wyP * vec4(wyPosition, 0.0, 1.0);
}


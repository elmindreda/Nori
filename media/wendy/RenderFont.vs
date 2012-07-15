
#version 150

in vec2 vPosition;
in vec2 vTexCoord;

out vec2 texCoord;

void main()
{
  texCoord = vTexCoord;

  gl_Position = wyP * vec4(vPosition, 0.0, 1.0);
}


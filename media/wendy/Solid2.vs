
uniform mat4 MVP;

in vec2 position;

void main()
{
  gl_Position = MVP * vec4(position, 0, 1);
}


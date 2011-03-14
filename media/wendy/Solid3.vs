
uniform mat4 MVP;

in vec3 position;

void main()
{
  gl_Position = MVP * vec4(position, 1);
}


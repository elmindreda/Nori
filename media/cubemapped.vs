
uniform mat4 MVP;
uniform mat4 MV;

attribute vec3 position;
attribute vec3 normal;

varying vec3 eyepos70;
varying vec3 normal70;

void main()
{
  eyepos70 = MV * vec4(position, 1);
  normal70 = MV * vec4(normal, 0);

  gl_Position = MVP * vec4(position, 1);
}


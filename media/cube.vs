
uniform mat4 wyMVP;
uniform mat4 wyMV;

attribute vec3 wyPosition;
attribute vec3 wyNormal;

varying vec3 viewpos;
varying vec3 normal;

void main()
{
  viewpos = (wyMV * vec4(wyPosition, 1)).xyz;
  normal = wyNormal;

  gl_Position = wyMVP * vec4(wyPosition, 1);
}


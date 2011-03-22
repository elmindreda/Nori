
attribute vec3 wyPosition;
attribute vec3 wyNormal;

varying vec3 normal;

void main()
{
  normal = (wyMV * vec4(wyNormal, 0)).xyz;

  gl_Position = wyMVP * vec4(wyPosition, 1);
}


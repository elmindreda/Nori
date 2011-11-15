
attribute vec3 wyPosition;
attribute vec3 wyNormal;

varying vec3 vPosition;
varying vec3 vNormal;

void main()
{
  vNormal = (wyMV * vec4(wyPosition, 1)).xyz;
  gl_Position = wyMVP * vec4(wyPosition, 1);
  vPosition = gl_Position.xyz;
}


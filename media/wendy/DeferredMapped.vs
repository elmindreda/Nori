
attribute vec3 wyPosition;
attribute vec3 wyNormal;
attribute vec2 wyTexCoord;

varying vec3 normal;
varying vec2 texCoord;

void main()
{
  normal = (wyMV * vec4(wyNormal, 0.0)).xyz;
  texCoord = wyTexCoord;

  gl_Position = wyMVP * vec4(wyPosition, 1.0);
}


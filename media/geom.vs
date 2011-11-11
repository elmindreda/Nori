
attribute vec3 wyPosition;
attribute vec3 wyNormal;

varying vec3 vViewpos;
varying vec3 vNormal;

void main()
{
  vViewpos = (wyMV * vec4(wyPosition, 1)).xyz;
  vNormal = wyNormal;

  gl_Position = wyMVP * vec4(wyPosition, 1);
}


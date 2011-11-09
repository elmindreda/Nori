
attribute vec3 wyPosition;
attribute vec3 wyNormal;

varying vec3 viewpos;
varying vec3 viewpos_geom;
varying vec3 normal;
varying vec3 normal_geom;

void main()
{
  viewpos = viewpos_geom = (wyMV * vec4(wyPosition, 1)).xyz;
  normal = normal_geom = wyNormal;

  gl_Position = wyMVP * vec4(wyPosition, 1);
}


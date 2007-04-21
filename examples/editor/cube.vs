
uniform float time;

varying vec3 N;
varying vec3 P;

void main()
{
  N = normalize(gl_NormalMatrix * gl_Normal).xyz;
  P = (gl_ModelViewMatrix * gl_Vertex).xyz;

  gl_FrontColor = gl_Color;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}



uniform float time;

varying vec3 N;
varying vec3 P;

void main()
{
  N = normalize(gl_NormalMatrix * gl_Normal).xyz;

  vec4 position = gl_ModelViewMatrix * gl_Vertex;
  position.y += sin(time);

  P = position.xyz;

  gl_FrontColor = gl_Color;
  gl_Position = gl_ProjectionMatrix * position;
}


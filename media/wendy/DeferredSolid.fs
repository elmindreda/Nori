
uniform vec3 color;
uniform float mask;

varying vec3 normal;

void main()
{
  gl_FragData[0] = vec4(color, 1.0);
  gl_FragData[1] = vec4(normalize(normal) / 2.0 + 0.5, mask);
}


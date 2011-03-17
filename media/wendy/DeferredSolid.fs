
uniform vec3 color;
uniform float mask;

in vec3 normal;

void main()
{
  gl_FragData[0] = vec4(color, 1);
  gl_FragData[1] = vec4(normalize(normal) / 2 + 0.5, mask);
}


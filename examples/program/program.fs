
varying vec3 normal;

uniform sampler2D program;

void main()
{
  vec2 texcoord = normalize(normal).xy * vec2(0.5) + vec2(0.5);
  vec3 color = texture2D(program, texcoord);
  gl_FragColor = vec4(color, 1.0);
}


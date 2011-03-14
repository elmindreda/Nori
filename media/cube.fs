
uniform sampler2D checkers;

const vec3 L = normalize(vec3(1, 0, 1));

varying vec3 viewpos;
varying vec3 normal;
varying vec2 mapping;

vec4 main()
{
  vec3 C = vec3(1.0, 0.3, 0.3) * texture2D(checkers, mapping).r;

  vec3 R = reflect(L, normal);
  vec3 V = viewpos;

  float Ia = 0.1;
  float Id = clamp(dot(normal, L), 0, 1);
  float Is = pow(clamp(dot(R, V), 0, 1), 15);

  gl_FragColor = vec4(C * (Ia + Id) + vec3(1) * Is, 1);
}



uniform vec3 color;

const vec3 L = normalize(vec3(1, 0, 1));

varying vec3 viewpos;
varying vec3 normal;

void main()
{
  vec3 C = color;

  vec3 R = reflect(L, normal);
  vec3 V = viewpos;

  float Ia = 0.1;
  float Id = clamp(dot(normal, L), 0.0, 1.0);
  float Is = pow(clamp(dot(R, V), 0.0, 1.0), 15.0);

  gl_FragColor = vec4(C * (Ia + Id) + Is, 1.0);
}


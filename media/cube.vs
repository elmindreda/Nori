
struct Vertex
{
  vec3 position;
  vec3 normal;
};

uniform mat4 MVP;
uniform mat4 MV;

in Vertex vertex;

varying vec3 viewpos;
varying vec3 normal;
varying vec2 mapping;

void main()
{
  vec3 viewpos = (MV * vec4(vertex.position, 1)).xyz;

  vec3 u = normalize(viewpos);
  vec3 n = (MV * vec4(vertex.normal, 0)).xyz;
  vec3 r = reflect(u, n);
  vec3 t = r + vec3(0, 0, 1);
  float p = sqrt(dot(t, t));

  viewpos = normalize(viewpos);
  normal = n;
  mapping = vec2(0.5 + r.x / (2 * p), 0.5 + r.y / (2 * p));

  gl_Position = MVP * vec4(vertex.position, 1);
}



uniform mat4 MVP;
uniform mat4 MV;

struct Vertex
{
  vec3 position;
  vec3 normal;
  vec2 mapping;
};

in Vertex vertex;

varying vec3 normal;
varying vec2 mapping;

void main()
{
  normal = (MV * vec4(vertex.normal, 0)).xyz;
  mapping = vertex.mapping;

  gl_Position = MVP * vec4(vertex.position, 1);
}


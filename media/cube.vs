
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

void main()
{
  viewpos = (MV * vec4(vertex.position, 1)).xyz;
  normal = vertex.normal;

  gl_Position = MVP * vec4(vertex.position, 1);
}


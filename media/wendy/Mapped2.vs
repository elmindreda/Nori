
struct Vertex
{
  vec2 position;
  vec2 mapping;
};

uniform mat4 MVP;

in Vertex vertex;

varying vec2 mapping;

void main()
{
  mapping = vertex.mapping;

  gl_Position = MVP * vec4(vertex.position, 0, 1);
}


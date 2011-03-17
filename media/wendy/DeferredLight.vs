
struct Vertex
{
  vec2 position;
  vec2 mapping;
  vec2 clipOverF;
};

uniform mat4 P;

in Vertex vertex;

varying vec2 mapping;
varying vec2 clipOverF;

void main()
{
  mapping = vertex.mapping;
  clipOverF = vertex.clipOverF;

  gl_Position = P * vec4(vertex.position, 0, 1);
}


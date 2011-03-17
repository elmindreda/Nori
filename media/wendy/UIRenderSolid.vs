
struct Vertex
{
  vec2 position;
};

uniform mat4 P;

in Vertex vertex;

void main()
{
  gl_Position = P * vec4(vertex.position, 0, 1);
}


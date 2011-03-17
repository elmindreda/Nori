
uniform mat4 MVP;

struct Vertex
{
  vec3 position;
};

in Vertex vertex;

void main()
{
  gl_Position = MVP * vec4(vertex.position, 1);
}


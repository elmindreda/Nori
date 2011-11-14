
in vec3 wyPosition;
in vec3 wyNormal;
out vec3 vPosition;
out vec3 vNormal;

void main()
{
  vPosition = wyPosition;
  vNormal = wyNormal;
}

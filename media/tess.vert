
in vec3 wyPosition;
in vec3 wyNormal;
out vec3 vPosition;
out vec3 vNormal;

void main()
{
  vPosition = (wyMV * vec4(wyPosition, 1)).xyz;
  vNormal = (wyMV * vec4(wyNormal, 0)).xyz;
}


uniform samplerCube cubemap;

varying vec3 eyevec;
varying vec3 normal;

void main()
{
  vec3 R = -reflect(normalize(eyevec), normalize(normal));

  gl_FragColor = textureCube(cubemap, R);
}


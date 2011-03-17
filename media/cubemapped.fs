
uniform samplerCube cubemap;

varying vec3 eyepos;
varying vec3 normal;

void main()
{
  vec3 R = normalize(reflect(-eyepos, normal));

  gl_FragColor = textureCube(cubemap, R);
}


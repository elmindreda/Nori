
uniform samplerCube cubemap;

varying vec3 eyepos70;
varying vec3 normal70;

void main()
{
  vec3 R = normalize(reflect(-eyepos70, normal70));

  gl_FragColor = textureCube(cubemap, R);
}


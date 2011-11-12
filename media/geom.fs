
varying vec3 gViewpos;
varying vec3 gNormal;
varying vec3 gBarycentric;

void main()
{
  const vec3 lightPos = vec3(10, 0, 0);

  vec3 V = gViewpos;
  vec3 N = normalize(gNormal);
  vec3 L = normalize(lightPos - V);

  float ambient = 0.4;
  float diffuse = clamp(dot(N, L), 0.0, 1.0);

  float c1 = gBarycentric.x;
  float c2 = gBarycentric.y;
  float c3 = gBarycentric.z;

  float grey = min(min(c1, c2), c3);

  gl_FragColor = vec4(vec3(grey,grey,grey) * (ambient + diffuse), 1.0);
}


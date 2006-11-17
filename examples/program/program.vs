
uniform float time;

varying vec3 normal;

void main()
{
  vec3 light = vec3(0.0, 0.0, 1.0);

  normal = normalize(gl_NormalMatrix * gl_Normal);

  vec4 position = ftransform();
  position.y += sin(position.x + position.z + time);

  gl_FrontColor = gl_Color * dot(light, normal);
  gl_Position = position;
}


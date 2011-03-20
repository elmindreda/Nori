
uniform sampler2D colormap;
uniform sampler2D normalmap;

varying vec3 normal;
varying vec2 texCoord;

void main()
{
  vec4 color = texture2D(colormap, texCoord);

  if (color.a - 0.5 < 0.0)
    discard;

  gl_FragData[0] = color;
  gl_FragData[1] = vec4(normalize(normal * sign(normal.z)) / 2.0 + 0.5, 0.0);
}


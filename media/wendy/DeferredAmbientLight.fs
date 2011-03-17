
struct Light
{
  vec3 color;
};

uniform Light light;
uniform sampler2DRect colorTexture;

varying vec2 mapping;
varying vec2 clipOverF;

void main()
{
  vec3 Cs = texture2DRect(colorTexture, mapping).rgb;

  gl_FragColor = vec4(Cs * light.color, 1);
}


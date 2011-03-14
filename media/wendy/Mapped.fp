
uniform sampler2D image;

struct VF
{
  vec2 mapping;
};

varying Varyings varyings;

void main()
{
  gl_FragColor = texture2D(image, mapping);
}


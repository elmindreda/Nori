
uniform sampler2D image;

in vec2 mapping;

void main()
{
  gl_FragColor = texture2D(image, mapping);
}


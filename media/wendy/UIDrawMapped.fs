
uniform sampler2D image;

in vec2 texCoord;

void main()
{
  gl_FragColor = texture2D(image, texCoord);
}



uniform sampler2D image;

varying vec2 texCoord;

void main()
{
  gl_FragColor = texture2D(image, texCoord);
}


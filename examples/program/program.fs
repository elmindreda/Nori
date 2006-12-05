
#pragma wendyLighting

varying vec3 N;
varying vec3 P;

void main()
{
  vec3 Nn = normalize(N);

  gl_FragColor = gl_Color * (vec4(ambient(P), 1.0) +
                             vec4(diffuse(P, Nn), 1.0)) +
		 vec4(specular(P, Nn, 5.0), 1.0);
}



uniform vec3 color;
in vec3 gNormal;
in vec3 gTriDistance;
in vec3 gPatchDistance;
out vec4 FragColor;

float amplify(float d, float scale, float offset)
{
    d = scale * d + offset;
    d = clamp(d, 0, 1);
    d = 1 - exp2(-2*d*d);
    return d;
}

void main()
{
    const vec3 LightPosition = vec3(0, 10, 0);

    vec3 N = normalize(gNormal);
    float df = abs(dot(N, LightPosition));
    vec3 c = vec3(0.1, 0.1, 0.1) + df * color;

    float d1 = min(min(gTriDistance.x, gTriDistance.y), gTriDistance.z);
    float d2 = min(min(gPatchDistance.x, gPatchDistance.y), gPatchDistance.z);
    c = amplify(d1, 40, -0.5) * amplify(d2, 60, -0.5) * c;

    FragColor = vec4(c, 1.0);
}

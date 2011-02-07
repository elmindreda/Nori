// Adapted from the RenderMonkey HDR sample

float2 gaussFilter[3];

float4 main(in float2 mapping, uniform sampler2D image, uniform float2 scale) : COLOR
{
  float4 color = float4(0.0);
  
  gaussFilter[0] = float2(-1.5, 15.0 / 50.0);
  gaussFilter[1] = float2( 0.0, 20.0 / 50.0);
  gaussFilter[2] = float2( 1.5, 15.0 / 50.0);
  
  for (int i = 0;  i < 3;  i++)
  {
    color += tex2D(image, float2(mapping.s + gaussFilter[i].s * scale.s, mapping.t)) * 
             gaussFilter[i].t;
  }
  
  return color;
}


// Adapted from the RenderMonkey HDR sample

float scale = 1.0 / 256.0;

float4 gaussFilter[7];

float4 main(in float2 mapping, uniform sampler2D image) : COLOR
{
  float4 color = float4(0.0);
  
  gaussFilter[0] = float4(-3.0, 0.0, 0.0,  5.0 / 54.0);
  gaussFilter[1] = float4(-2.0, 0.0, 0.0,  7.0 / 54.0);
  gaussFilter[2] = float4(-1.0, 0.0, 0.0, 10.0 / 54.0);
  gaussFilter[3] = float4( 0.0, 0.0, 0.0, 15.0 / 54.0);
  gaussFilter[4] = float4( 1.0, 0.0, 0.0, 10.0 / 54.0);
  gaussFilter[5] = float4( 2.0, 0.0, 0.0,  7.0 / 54.0);
  gaussFilter[6] = float4( 3.0, 0.0, 0.0,  5.0 / 64.0);
  
  for (int i = 0;  i < 7;  i++)
  {
    color += tex2D(image, float2(mapping.s + gaussFilter[i].s * scale,
                                 mapping.t + gaussFilter[i].t * scale)) * 
             gaussFilter[i].q;
  }
  
  return color;
}


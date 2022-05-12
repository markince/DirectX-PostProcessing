//--------------------------------------------------------------------------------------
// Full screen Blur Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Samples a pixel from the scene texture and blurs it

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture, these variables allow access to that texture
Texture2D    SceneTexture : register(t0);
SamplerState PointSample  : register(s0); // We don't usually want to filter (bilinear, trilinear etc.) the scene texture when
										  // post-processing so this sampler will use "point sampling" - no filtering

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Post-processing shader that blurs a pixel
float4 main(PostProcessingInput input) : SV_Target
{
    float3 colour = SceneTexture.Sample(PointSample, input.sceneUV).rgb;

    return float4(colour, 0.1f);
}


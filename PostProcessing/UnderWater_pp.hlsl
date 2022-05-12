//--------------------------------------------------------------------------------------
// Under Water Post-Processing pixel Shader
//--------------------------------------------------------------------------------------
// Samples a pixel from the scene texture and makes an underwater effect

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture, these variables allow access to that texture
Texture2D SceneTexture   : register(t0);
SamplerState PointSample : register(s0); // We don't usually want to filter (bilinear, trilinear etc.) the scene texture when
										 // post-processing so this sampler will use "point sampling" - no filtering

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Post-processing shader that tints the scene texture to a given colour

float4 main(PostProcessingInput input) : SV_Target
{
    const float  waterEffectStrength = 0.005f;
    const float3 blueColour          = float3(0.2f, 0.2f, 1.0f);

    // The water effect is a combination of sine waves in x and y dimensions
    float SinX = sin(input.sceneUV.x * radians(1440.0f) + gUnderWaterLevel * 3.0f);
    float SinY = sin(input.sceneUV.y * radians(3600.0f) + gUnderWaterLevel * 3.7f);

    // Offset for scene texture UV based on under water effect
    // Adjust size of UV offset based on the constant EffectStrength, 
    // the overall size of area being processed, and the alpha value calculated above
    float2 waterOffset = float2(SinY, SinX) * waterEffectStrength;

    // Get pixel from scene texture, offset using waterOffset
    float3 finalPixelColour = SceneTexture.Sample(PointSample, input.sceneUV + waterOffset).rgb;

    // Add a blue water colour effect
    finalPixelColour = finalPixelColour * blueColour;

    return float4(finalPixelColour, 1.0f);

}
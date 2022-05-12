//--------------------------------------------------------------------------------------
// Spiral Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Makes the screen spiral round using complex COS mathematics!

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

// Post-processing shader that tints the scene texture to a given colour
float4 main(PostProcessingInput input) : SV_Target
{
	// Get vector from screen centre to pixel UV
    const float2 centreUV = float2(0.5f, 0.5f);
    float2 centreOffsetUV = float2(input.sceneUV.x, input.sceneUV.y) - centreUV;
    float centreDistance = length(centreOffsetUV); // Distance of pixel from screen centre
	
	// Get sin and cos of spiral amount, increasing with distance from centre
    float s, c;
    sincos(centreDistance * gSpiralLevel * gSpiralLevel, s, c);
	
	// Create a (2D) rotation matrix and apply to the vector - i.e. rotate the
	// vector around the centre by the spiral amount
    matrix<float, 2, 2> rot2D =
    {
       c, s,
	  -s, c
    };
    
    float2 rotatedOffsetUV = mul(centreOffsetUV, rot2D);

	// Sample texture at new position (centre UV + rotated UV offset)
    float3 outputColour = SceneTexture.Sample(PointSample, centreUV + rotatedOffsetUV).rgb;

    return float4(outputColour, 1.0f);
}
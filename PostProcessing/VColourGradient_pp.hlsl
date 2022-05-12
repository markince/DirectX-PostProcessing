//--------------------------------------------------------------------------------------
// Vertical colour gradient Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// A colour tint all over the whole screen where the tint colour varies from the top of the screen to the bottom

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

float4 main(PostProcessingInput input) : SV_Target
{
    float distance = input.sceneUV.y;

	float3 colourBlue   = float3(1.0f, 0.0f, 0.0f);
	float3 colourYellow = float3(0.0f, 0.0f, 1.0f);

	colourBlue   = colourBlue * (1.0f - distance);
	colourYellow = colourYellow * distance;

    float3 finalPixelColour = SceneTexture.Sample(PointSample, input.sceneUV).rgb;
	finalPixelColour.rgb = finalPixelColour.rbg * (colourYellow + colourBlue);

	return float4(finalPixelColour, 1.0f);

}
//--------------------------------------------------------------------------------------
// Grey Noise Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// 

#include "Common.hlsli"


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture, these variables allow access to that texture
Texture2D    SceneTexture : register(t0);
SamplerState PointSample  : register(s0); // We don't usually want to filter (bilinear, trilinear etc.) the scene texture when
                                          // post-processing so this sampler will use "point sampling" - no filtering

// This shader also uses a texture filled with noise
Texture2D    NoiseMap      : register(t1);
SamplerState TrilinearWrap : register(s1);


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{
    const float NoiseStrength = 0.5f; // How noticable the noise is

	// Get scene pixel colour and average r, g & b to get a single grey value
    float3 sceneColour = SceneTexture.Sample(PointSample, input.sceneUV).rgb;
    float grey = (sceneColour.r + sceneColour.g + sceneColour.b) / 3.0f;

	// Get noise UV by scaling and offseting scene texture UV. Scaling adjusts how fine the noise is.
	// The offset is randomised every frame (in C++) to give a constantly changing noise effect (like tv static)
    float2 noiseUV = input.sceneUV * gNoiseScale + gNoiseOffset;
    grey += NoiseStrength * (NoiseMap.Sample(TrilinearWrap, noiseUV).r - 0.5f); // Noise can increase or decrease grey value hence the -0.5f

	// Calculate alpha to display the effect in a softened circle, could use a texture rather than calculations for the same task.
	// Uses the second set of area texture coordinates, which range from (0,0) to (1,1) over the area being processed
    float softEdge = 0.20f; // Softness of the edge of the circle - range 0.001 (hard edge) to 0.25 (very soft)
    float2 centreVector = input.areaUV - float2(0.5f, 0.5f);
    float centreLengthSq = dot(centreVector, centreVector);
    float alpha = 1.0f - saturate((centreLengthSq - 0.25f + softEdge) / softEdge); // Soft circle calculation based on fact that this circle has a radius of 0.5 (as area UVs go from 0->1)
																					   
	// Output final colour
    return float4(grey, grey, grey, alpha);
}
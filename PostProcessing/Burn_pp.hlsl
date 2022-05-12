//--------------------------------------------------------------------------------------
// Burn Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
// Burn screen effect

#include "Common.hlsli"


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture, these variables allow access to that texture
Texture2D    SceneTexture   : register(t0);
SamplerState PointSample    : register(s0); // We don't usually want to filter (bilinear, trilinear etc.) the scene texture when
                                          // post-processing so this sampler will use "point sampling" - no filtering

// This shader also uses a "burn" texture, which is basically a height map that the burn level ascends
Texture2D    BurnMap          : register(t1);
SamplerState TrilinearWrap    : register(s1);


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Post-processing shader that tints the scene texture to a given colour
float4 main(PostProcessingInput input) : SV_Target
{
    const float3 white = 1.0f;
	
	// Pixels are burnt with these colours at the edges
    const float3 burnColour = float3(0.8f, 0.4f, 0.0f);
    const float3 glowColour = float3(1.0f, 0.8f, 0.0f);
    const float  glowAmount = 0.25f; // Thickness of glowing area
    const float  crinkle = 0.15f; // Amount of texture crinkle at the edges 

	// Get burn texture colour
    float4 burnTexture = BurnMap.Sample(TrilinearWrap, input.areaUV);
    
    // The range of burning colours are from gBurnLevel  to burnLevelMax
    float burnLevelMax = gBurnHeight + glowAmount;

    float3 outputColour;

	// Output black when current burn texture value below burning range
    if (burnTexture.r <= gBurnHeight)
    {
        outputColour = float3(0.0f, 0.0f, 0.0f);
    }
    
    // Output scene texture untouched when current burnTexture texture value above burning range
    else if (burnTexture.r >= burnLevelMax)
    {
        outputColour = SceneTexture.Sample(PointSample, input.sceneUV).rgb;
    }
    else // Draw burning edges
    {
		// Get level of glow (0 = none, 1 = max)
        float glowLevel = 1.0f - (burnTexture.r - gBurnHeight) / glowAmount;

		// Extract direction to crinkle (2D vector) from the g & b components of the burn texture sampled above
        float2 crinkleVector = normalize(burnColour.b - burnColour.g);
		
		// Convert from UV 0->1 range to -0.5->0.5 range (to give vector in any direction)
        crinkleVector -= float2(0.5f, 0.5f);

		// Get main texture colour using crinkle offset
        float3 texColour = SceneTexture.Sample(PointSample, input.sceneUV - glowLevel * crinkle * crinkleVector).rgb;

		// Split glow into two regions - the very edge and the inner section
        glowLevel *= 2.0f;
        if (glowLevel < 1.0f)
        {
			// Blend from main texture colour on inside to burn tint in middle of burning area
            outputColour = lerp(texColour, burnColour * texColour, glowLevel);
        }
        else
        {
			// Blend from burn tint in middle of burning area to bright glow at the burning edges
            outputColour = lerp(burnColour * texColour, glowColour, glowLevel - 1.0f);
        }
    }

    return float4(outputColour, 0.4f);
}
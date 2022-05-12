//--------------------------------------------------------------------------------------
// Bloom Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
//  Creates a bloom effect
// Ref:
// https://www.shadertoy.com/view/lsXGWn

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

Texture2D    SceneTexture : register(t0);
SamplerState PointSample  : register(s0);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float4 main(PostProcessingInput input) : SV_Target
{

    const float blurSize = 1.0f / 512.0f;
    const float intensity = 0.5f;
    
    float2 texcoord = input.sceneUV;
    float3 sum = float3(0, 0, 0);
    
    // Blur in y (Vertical)
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x - 4.0 * blurSize, texcoord.y)).rgb * 0.05;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x - 3.0 * blurSize, texcoord.y)).rgb * 0.09;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x - 2.0 * blurSize, texcoord.y)).rgb * 0.12;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x - blurSize, texcoord.y)).rgb * 0.15;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x, texcoord.y)).rgb * 0.16;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x + blurSize, texcoord.y)).rgb * 0.15;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x + 2.0 * blurSize, texcoord.y)).rgb * 0.12;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x + 3.0 * blurSize, texcoord.y)).rgb * 0.09;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x + 4.0 * blurSize, texcoord.y)).rgb * 0.05;
    
    // Blur in x (Horizontal)
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x, texcoord.y - 4.0 * blurSize)).rgb * 0.05;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x, texcoord.y - 3.0 * blurSize)).rgb * 0.09;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x, texcoord.y - 2.0 * blurSize)).rgb * 0.12;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x, texcoord.y - blurSize)).rgb * 0.15;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x, texcoord.y)).rgb * 0.16;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x, texcoord.y + blurSize)).rgb * 0.15;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x, texcoord.y + 2.0 * blurSize)).rgb * 0.12;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x, texcoord.y + 3.0 * blurSize)).rgb * 0.09;
    sum += SceneTexture.Sample(PointSample, float2(texcoord.x, texcoord.y + 4.0 * blurSize)).rgb * 0.05;
    
    // Increase blur with intensity
    
	// Calculate final bloom effect
    float3 finalBloom = sum * intensity;
    
    // Sample a pixel from the scene texture and apply bloom
    float3 finalPixelColour = finalBloom + SceneTexture.Sample(PointSample, input.sceneUV).rgb;
    
    return float4(finalPixelColour, 1.0f);
}
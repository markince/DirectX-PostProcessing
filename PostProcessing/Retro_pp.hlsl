//--------------------------------------------------------------------------------------
// Retro game mode post-processing Pixel Shader 
//--------------------------------------------------------------------------------------
// Makes pixels look much larger and uses a limited colour pallet, but still shoes the 
// scene being rendered. This gives a 80's retro game mode effect
// Ref - https://gamedev.stackexchange.com/questions/111017/pixelation-shader-explanation
// Also uses a limited colour palette as per the assignment specification
// Ref - https://gamedev.stackexchange.com/questions/49454/how-can-i-replicate-the-color-limitations-of-the-nes-with-an-hlsl-pixel-shader

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture, these variables allow access to that texture
Texture2D SceneTexture   : register(t0);
SamplerState PointSample : register(s0); // We don't usually want to filter (bilinear, trilinear etc.) the scene texture when
										 // post-processing so this sampler will use "point sampling" - no filtering

SamplerState TrilinearWrap : register(s1);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float3 LimitColour(float3 colour)
{
    const float oneOver7 = 1.0 / 8.0;
    const float oneOver3 = 1.0 / 3.0;

    float R = floor(colour.r * 7.99) * oneOver7;
    float G = floor(colour.g * 7.99) * oneOver7;
    float B = floor(colour.b * 3.99) * oneOver3;
    
    return float3(R, G, B);
}

float4 main(PostProcessingInput input) : SV_Target
{
    float pixels = gViewportHeight / 30.0f * gViewportWidth / 30.0f;
	
    float dx = 5.0f * (1.0f / pixels);
    float dy = 5.0f * (1.0f / pixels);

    float2 coords = float2(dx * floor(input.sceneUV.x / dx), dy * floor(input.sceneUV.y / dy));

    float3 finalPixelColour = SceneTexture.Sample(PointSample, coords);
    
    finalPixelColour = LimitColour(finalPixelColour);
	
    return float4(finalPixelColour, 1.0f);
}

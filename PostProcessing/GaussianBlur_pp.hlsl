//--------------------------------------------------------------------------------------
// Gaussian Blur Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader Two pass Gaussian blur effect

#include "Common.hlsli" // Shaders can also use include files - note the extension

// Ref: https://stackoverflow.com/questions/36303950/hlsl-gaussian-blur-effect

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------


Texture2D    SceneTexture : register(t0); 
SamplerState TexSampler   : register(s0); 

SamplerState TriSample : register(s1);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader entry point - each shader has a "main" function
// This shader just samples a diffuse texture map
float4 main(PostProcessingInput input) : SV_Target
{
    // Final pixel colour
    float4 finalPixelColour = { 0.0f, 0.0f, 0.0f, 1.0f };
    
    // Direction of the blur
    float horizontalStep = gBlurDirection.x; 
    float verticalStep   = gBlurDirection.y; 
    
    const float kernelOffsets[3] = { 0.0f,          1.3846153846f, 3.2307692308f };
    const float blurWeights[3]   = { 0.2270270270f, 0.3162162162f, 0.0702702703f };
    
    if (horizontalStep > 0.0f)
    {
        finalPixelColour.rgb = SceneTexture.Sample(TriSample, input.sceneUV).rgb * blurWeights[0];
        
        for (int i = 1; i < 3; i++)
        {
            float2 normalizedOffset = float2(kernelOffsets[i], 0.0f) / gViewportWidth;
            finalPixelColour.rgb += SceneTexture.Sample(TriSample, input.sceneUV + normalizedOffset).xyz * blurWeights[i];
            finalPixelColour.rgb += SceneTexture.Sample(TriSample, input.sceneUV - normalizedOffset).xyz * blurWeights[i];
        }
    }
    else
    {
        finalPixelColour.rgb = SceneTexture.Sample(TriSample, input.sceneUV).rgb * blurWeights[0];
        
        for (int i = 1; i < 3; i++)
        {
            float2 normalizedOffset = float2(0.0f, kernelOffsets[i]) / gViewportHeight;
            finalPixelColour.rgb += SceneTexture.Sample(TriSample, input.sceneUV + normalizedOffset).xyz * blurWeights[i];
            finalPixelColour.rgb += SceneTexture.Sample(TriSample, input.sceneUV - normalizedOffset).xyz * blurWeights[i];
        }
    }
    
    return finalPixelColour;
}
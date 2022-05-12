//--------------------------------------------------------------------------------------
// Vertical HSL colour gradient post-processing Pixel Shader 
//--------------------------------------------------------------------------------------
// Colour tints a pixel over the whole screen where the tint colour varies from the top of the screen to the bottom
// Updated version using HSL colour space
// Ref - https://gist.github.com/mairod/a75e7b44f68110e1576d77419d608786

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// The scene has been rendered to a texture, these variables allow access to that texture
Texture2D SceneTexture : register(t0);
SamplerState PointSample : register(s0); // We don't usually want to filter (bilinear, trilinear etc.) the scene texture when
										 // post-processing so this sampler will use "point sampling" - no filtering

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float3 ShiftHue(float3 colour, float hueShift)
{
    const float3 k = float3(0.57735f, 0.57735f, 0.57735f);
    float cosAngle = cos(hueShift);
    return (colour * cosAngle + cross(k, colour) * sin(hueShift) + k * dot(k, colour) * (1.0f - cosAngle));

}

float4 main(PostProcessingInput input) : SV_Target
{
    const float gradiantTintStrength = 4.0f;


    float hueShift = gHueShift;
    float distance = input.sceneUV.y;

    float3 gradiantColour1 = float3(1.0f, 0.0f, 0.0f);
    float3 gradiantColour2 = float3(0.0f, 0.0f, 1.0f);

    gradiantColour1 = gradiantColour1 * (1.0f - distance);
    gradiantColour2 = gradiantColour2 * distance;

    gradiantColour1 = ShiftHue(gradiantColour1, hueShift);
    gradiantColour2 = ShiftHue(gradiantColour2, hueShift);

    float3 finalPixelColour = SceneTexture.Sample(PointSample, input.sceneUV).rgb;

    finalPixelColour.rbg = finalPixelColour.rbg * (gradiantColour1 + gradiantColour2) * gradiantTintStrength;
	
    return float4(finalPixelColour, 1.0f);
}
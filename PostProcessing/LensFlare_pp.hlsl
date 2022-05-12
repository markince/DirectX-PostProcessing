//--------------------------------------------------------------------------------------
// Lens-Flare Post-Processing Pixel Shader
//--------------------------------------------------------------------------------------
//  Creates a lens flare effect
// Ref:
// https://www.shadertoy.com/view/4sX3Rs

#include "Common.hlsli"

//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

Texture2D    SceneTexture : register(t0);
SamplerState PointSample  : register(s0);

//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

float noise(float t)
{
    return texture(iChannel0, float2(t, .0) / iChannelResolution[0].xy).x;
}

float noise(float2 t)
{
    return texture(iChannel0, t / iChannelResolution[0].xy).x;
}

float3 lensflare(float2 uv, float2 pos)
{
    float2 main = uv - pos;
    float2 uvd = uv * (length(uv));
	
    float ang = atan2(main.x, main.y);
    float dist = length(main);
    dist = pow(dist, 0.1f);
    float n = noise(float2(ang * 16.0, dist * 32.0));
	
    float f0 = 1.0 / (length(uv - pos) * 16.0 + 1.0);
	
    f0 = f0 + f0 * (sin(noise(sin(ang * 2. + pos.x) * 4.0 - cos(ang * 3. + pos.y)) * 16.) * .1 + dist * .1 + .8);
	
    float f1 = max(0.01 - pow(length(uv + 1.2 * pos), 1.9), .0) * 7.0;

    float f2 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.8 * pos), 2.0)), .0) * 0.25f;
    float f22 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.85 * pos), 2.0)), .0) * 0.23f;
    float f23 = max(1.0 / (1.0 + 32.0 * pow(length(uvd + 0.9 * pos), 2.0)), .0) * 0.21f;
	
    float2 uvx = mix(uv, uvd, -0.5f);
	
    float f4 = max(0.01 - pow(length(uvx + 0.4 * pos), 2.4), .0) * 6.0;
    float f42 = max(0.01 - pow(length(uvx + 0.45 * pos), 2.4), .0) * 5.0;
    float f43 = max(0.01 - pow(length(uvx + 0.5 * pos), 2.4), .0) * 3.0;
	
    uvx = mix(uv, uvd, -.4);
	
    float f5 = max(0.01 - pow(length(uvx + 0.2 * pos), 5.5), .0) * 2.0;
    float f52 = max(0.01 - pow(length(uvx + 0.4 * pos), 5.5), .0) * 2.0;
    float f53 = max(0.01 - pow(length(uvx + 0.6 * pos), 5.5), .0) * 2.0;
	
    uvx = mix(uv, uvd, -0.5);
	
    float f6 = max(0.01 - pow(length(uvx - 0.3 * pos), 1.6), .0) * 6.0;
    float f62 = max(0.01 - pow(length(uvx - 0.325 * pos), 1.6), .0) * 3.0;
    float f63 = max(0.01 - pow(length(uvx - 0.35 * pos), 1.6), .0) * 5.0;
	
    float3 c = float3(.0);
	
    c.r += f2 + f4 + f5 + f6;
    c.g += f22 + f42 + f52 + f62;
    c.b += f23 + f43 + f53 + f63;
    c = c * 1.3 - float3(length(uvd) * .05);
    c += float3(f0);
	
    return c;
}

float3 cc(float3 color, float factor, float factor2) // color modifier
{
    float w = color.x + color.y + color.z;
    return mix(color, float3(w) * factor, w * factor2);
}

void mainImage(out float4 fragColor, in float2 fragCoord)
{
    float2 uv = fragCoord.xy / iResolution.xy - 0.5;
    uv.x *= iResolution.x / iResolution.y; //fix aspect ratio
    float3 mouse = vec3(iMouse.xy / iResolution.xy - 0.5, iMouse.z - .5);
    mouse.x *= iResolution.x / iResolution.y; //fix aspect ratio
    if (iMouse.z < .5)
    {
        mouse.x = sin(iTime) * .5;
        mouse.y = sin(iTime * .913) * .5;
    }
	
    float3 color = float3(1.4, 1.2, 1.0) * lensflare(uv, mouse.xy);
    color -= noise(fragCoord.xy) * .015;
    color = cc(color, .5, .1);
    fragColor = float4(color, 1.0);
}
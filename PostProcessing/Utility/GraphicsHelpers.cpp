//--------------------------------------------------------------------------------------
// Helper functions to unclutter and simplify code elsewhere
//--------------------------------------------------------------------------------------

#include "GraphicsHelpers.h"
#include "../Shader.h"
#include "../Common.h"

#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include <cmath>
#include <cctype>
#include <atlbase.h> // C-string to unicode conversion function CA2CT

//--------------------------------------------------------------------------------------
// Texture Loading
//--------------------------------------------------------------------------------------

// Using Microsoft's open source DirectX Tool Kit (DirectXTK) to simplify texture loading
// This function requires you to pass a ID3D11Resource* (e.g. &gTilesDiffuseMap), which manages the GPU memory for the
// texture and also a ID3D11ShaderResourceView* (e.g. &gTilesDiffuseMapSRV), which allows us to use the texture in shaders
// The function will fill in these pointers with usable data. Returns false on failure
bool LoadTexture(std::string filename, ID3D11Resource** texture, ID3D11ShaderResourceView** textureSRV)
{
    // DDS files need a different function from other files
    std::string dds = ".dds"; // So check the filename extension (case insensitive)
    if (filename.size() >= 4 &&
        std::equal(dds.rbegin(), dds.rend(), filename.rbegin(), [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); }))
    {
        return SUCCEEDED(DirectX::CreateDDSTextureFromFile(gD3DDevice, CA2CT(filename.c_str()), texture, textureSRV));
    }
    else
    {
        return SUCCEEDED(DirectX::CreateWICTextureFromFile(gD3DDevice, gD3DContext, CA2CT(filename.c_str()), texture, textureSRV));
    }
}


//--------------------------------------------------------------------------------------
// Camera Helpers
//--------------------------------------------------------------------------------------

// A "projection matrix" contains properties of a camera. Covered mid-module - the maths is an optional topic (not examinable).
// - Aspect ratio is screen width / height (like 4:3, 16:9)
// - FOVx is the viewing angle from left->right (high values give a fish-eye look),
// - near and far clip are the range of z distances that can be rendered
CMatrix4x4 MakeProjectionMatrix(float aspectRatio /*= 4.0f / 3.0f*/, float FOVx /*= ToRadians(60)*/,
                                float nearClip /*= 0.1f*/, float farClip /*= 10000.0f*/)
{
    float tanFOVx = std::tan(FOVx * 0.5f);
    float scaleX = 1.0f / tanFOVx;
    float scaleY = aspectRatio / tanFOVx;
    float scaleZa = farClip / (farClip - nearClip);
    float scaleZb = -nearClip * scaleZa;

    return CMatrix4x4{ scaleX,   0.0f,    0.0f,   0.0f,
                         0.0f, scaleY,    0.0f,   0.0f,
                         0.0f,   0.0f, scaleZa,   1.0f,
                         0.0f,   0.0f, scaleZb,   0.0f };
}

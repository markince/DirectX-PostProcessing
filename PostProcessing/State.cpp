//--------------------------------------------------------------------------------------
// State creation
// - Sampler state (Bilinear, trilinear etc.)
// - Blender state (Additive blending, alpha blending etc.)
// - Rasterizer state (Wireframe mode, don't cull back faces etc.)
// - Depth stencil state (How to use the depth and stencil buffer)
//--------------------------------------------------------------------------------------

#include "State.h"
#include "Common.h"


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Globals used to keep code simpler, but try to architect your own code in a better way
//**** Update State.h if you add things here ****//

// GPU "States" //

// A sampler state object represents a way to filter textures, such as bilinear or trilinear. We have one object for each method we want to use
ID3D11SamplerState* gPointSampler         = nullptr;
ID3D11SamplerState* gTrilinearSampler     = nullptr;
ID3D11SamplerState* gAnisotropic4xSampler = nullptr;

// Blend states allow us to switch between blending modes (none, additive, multiplicative etc.)
ID3D11BlendState* gNoBlendingState       = nullptr;
ID3D11BlendState* gAdditiveBlendingState = nullptr;
ID3D11BlendState* gAlphaBlendingState    = nullptr;


// Rasterizer states affect how triangles are drawn
ID3D11RasterizerState* gCullBackState  = nullptr;
ID3D11RasterizerState* gCullFrontState = nullptr;
ID3D11RasterizerState* gCullNoneState  = nullptr;

// Depth-stencil states allow us change how the depth buffer is used
ID3D11DepthStencilState* gUseDepthBufferState = nullptr;
ID3D11DepthStencilState* gDepthReadOnlyState  = nullptr;
ID3D11DepthStencilState* gNoDepthBufferState  = nullptr;

//--------------------------------------------------------------------------------------
// State creation / destruction
//--------------------------------------------------------------------------------------

// Create all the states used in this app, returns true on success
bool CreateStates()
{
    //--------------------------------------------------------------------------------------
    // Texture Samplers
    //--------------------------------------------------------------------------------------
    // Each block of code creates a filtering mode. Copy a block and adjust values to add another mode. See texturing lab for details
    D3D11_SAMPLER_DESC samplerDesc = {};

    ////-------- Point Sampling (pixelated textures) --------////
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT; // Point filtering
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;  // Clamp addressing mode for texture coordinates outside 0->1
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;  // --"--
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;  // --"--
    samplerDesc.MaxAnisotropy = 1;                       // Number of samples used if using anisotropic filtering, more is better but max value depends on GPU

    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Controls how much mip-mapping can be used. These settings are full mip-mapping, the usual values
    samplerDesc.MinLOD = 0;                 // --"--

    // Then create a DirectX object for your description that can be used by a shader
    if (FAILED(gD3DDevice->CreateSamplerState(&samplerDesc, &gPointSampler)))
    {
        gLastError = "Error creating point sampler";
        return false;
    }


    ////-------- Trilinear Sampling --------////
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // Point filtering
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;   // Wrap addressing mode for texture coordinates outside 0->1
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;   // --"--
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;   // --"--
    samplerDesc.MaxAnisotropy = 1;                       // Number of samples used if using anisotropic filtering, more is better but max value depends on GPU

    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Controls how much mip-mapping can be used. These settings are full mip-mapping, the usual values
    samplerDesc.MinLOD = 0;                 // --"--

    // Then create a DirectX object for your description that can be used by a shader
    if (FAILED(gD3DDevice->CreateSamplerState(&samplerDesc, &gTrilinearSampler)))
    {
        gLastError = "Error creating point sampler";
        return false;
    }


    ////-------- Anisotropic filtering --------////
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC; // Trilinear filtering
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;    // Wrap addressing mode for texture coordinates outside 0->1
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;    // --"--
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;    // --"--
    samplerDesc.MaxAnisotropy = 4;                        // Number of samples used if using anisotropic filtering, more is better but max value depends on GPU

    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Controls how much mip-mapping can be used. These settings are full mip-mapping, the usual values
    samplerDesc.MinLOD = 0;                 // --"--

    // Then create a DirectX object for your description that can be used by a shader
    if (FAILED(gD3DDevice->CreateSamplerState(&samplerDesc, &gAnisotropic4xSampler)))
    {
        gLastError = "Error creating anisotropic 4x sampler";
        return false;
    }


    //--------------------------------------------------------------------------------------
    // Rasterizer States
    //--------------------------------------------------------------------------------------
    // Rasterizer states adjust how triangles are filled in and when they are shown
    // Each block of code creates a rasterizer state. Copy a block and adjust values to add another mode
    D3D11_RASTERIZER_DESC rasterizerDesc = {};

    ////-------- Back face culling --------////
    // This is the usual mode - don't show inside faces of objects
    rasterizerDesc.FillMode = D3D11_FILL_SOLID; // Can also set this to wireframe - experiment if you wish
    rasterizerDesc.CullMode = D3D11_CULL_BACK;  // Setting that decides whether the "front" and "back" side of each
                                                             // triangle is drawn or not. Culling back faces is the norm
    rasterizerDesc.DepthClipEnable = TRUE; // Advanced setting - only used in rare cases

    // Create a DirectX object for the description above that can be used by a shader
    if (FAILED(gD3DDevice->CreateRasterizerState(&rasterizerDesc, &gCullBackState)))
    {
        gLastError = "Error creating cull-back state";
        return false;
    }


    ////-------- Front face culling --------////
    // This is an unusual mode - it shows inside faces only so the model looks inside-out
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_FRONT; // Remove front faces
    rasterizerDesc.DepthClipEnable = TRUE; // Advanced setting - only used in rare cases

    // Create a DirectX object for the description above that can be used by a shader
    if (FAILED(gD3DDevice->CreateRasterizerState(&rasterizerDesc, &gCullFrontState)))
    {
        gLastError = "Error creating cull-front state";
        return false;
    }


    ////-------- No culling --------////
    // Used for transparent or flat objects - show both sides of faces
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;  // Don't cull any faces
    rasterizerDesc.DepthClipEnable = TRUE; // Advanced setting - only used in rare cases

    // Create a DirectX object for the description above that can be used by a shader
    if (FAILED(gD3DDevice->CreateRasterizerState(&rasterizerDesc, &gCullNoneState)))
    {
        gLastError = "Error creating cull-none state";
        return false;
    }


    //--------------------------------------------------------------------------------------
    // Blending States
    //--------------------------------------------------------------------------------------
    // Each block of code creates a filtering mode. Copy a block and adjust values to add another mode. See blending lab for details
    D3D11_BLEND_DESC blendDesc = {};

    ////-------- Blending Off State --------////
    blendDesc.RenderTarget[0].BlendEnable = FALSE;              // Disable blending
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;      // How to blend the source (texture colour)
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;     // How to blend the destination (colour already on screen)
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;   // How to combine the above two, almost always ADD

    //** Leave the following settings alone, they are used only in highly unusual cases
    //** Despite the word "Alpha" in the variable names, these are not the settings used for alpha blending
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    // Then create a DirectX object for the description that can be used by a shader
    if (FAILED(gD3DDevice->CreateBlendState(&blendDesc, &gNoBlendingState)))
    {
        gLastError = "Error creating no-blend state";
        return false;
    }


    ////-------- Additive Blending State --------////
    blendDesc.RenderTarget[0].BlendEnable = TRUE;             // Enable blending
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;    // How to blend the source (texture colour)
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;    // How to blend the destination (colour already on screen)
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // How to combine the above two, almost always ADD

    //** Leave the following settings alone, they are used only in highly unusual cases
    //** Despite the word "Alpha" in the variable names, these are not the settings used for alpha blending
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    // Then create a DirectX object for the description that can be used by a shader
    if (FAILED(gD3DDevice->CreateBlendState(&blendDesc, &gAdditiveBlendingState)))
    {
        gLastError = "Error creating additive blending state";
        return false;
    }


    ////-------- Alpha Blending State --------////
    blendDesc.RenderTarget[0].BlendEnable = TRUE;             // Enable blending
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;    // How to blend the source (texture colour)
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;    // How to blend the destination (colour already on screen)
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; // How to combine the above two, almost always ADD

    //** Leave the following settings alone, they are used only in highly unusual cases
    //** Despite the word "Alpha" in the variable names, these are not the settings used for alpha blending
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    // Then create a DirectX object for the description that can be used by a shader
    if (FAILED(gD3DDevice->CreateBlendState(&blendDesc, &gAlphaBlendingState)))
    {
        gLastError = "Error creating additive blending state";
        return false;
    }


    //--------------------------------------------------------------------------------------
    // Depth-Stencil States
    //--------------------------------------------------------------------------------------
    // Depth-stencil states adjust how the depth and stencil buffers are used. The stencil buffer is rarely used so 
    // these states are most often used to switch the depth buffer on and off. See depth buffers lab for details
    // Each block of code creates a rasterizer state. Copy a block and adjust values to add another mode
    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};

    ////-------- Enable depth buffer --------////
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = FALSE;

    // Create a DirectX object for the description above that can be used by a shader
    if (FAILED(gD3DDevice->CreateDepthStencilState(&depthStencilDesc, &gUseDepthBufferState)))
    {
        gLastError = "Error creating use-depth-buffer state";
        return false;
    }


    ////-------- Enable depth buffer reads only --------////
    // Disables writing to depth buffer - used for transparent objects because they should not be entered in the buffer but do need to check if they are behind something
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Disable writing to depth buffer
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = FALSE;

    // Create a DirectX object for the description above that can be used by a shader
    if (FAILED(gD3DDevice->CreateDepthStencilState(&depthStencilDesc, &gDepthReadOnlyState)))
    {
        gLastError = "Error creating depth-read-only state";
        return false;
    }


    ////-------- Disable depth buffer --------////
    depthStencilDesc.DepthEnable = FALSE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthStencilDesc.StencilEnable = FALSE;

    // Create a DirectX object for the description above that can be used by a shader
    if (FAILED(gD3DDevice->CreateDepthStencilState(&depthStencilDesc, &gNoDepthBufferState)))
    {
        gLastError = "Error creating no-depth-buffer state";
        return false;
    }

    return true;
}


// Release DirectX state objects
void ReleaseStates()
{
    if (gUseDepthBufferState)   gUseDepthBufferState   ->Release();
    if (gDepthReadOnlyState)    gDepthReadOnlyState    ->Release();
    if (gNoDepthBufferState)    gNoDepthBufferState    ->Release();
    if (gCullBackState)         gCullBackState         ->Release();
    if (gCullFrontState)        gCullFrontState        ->Release();
    if (gCullNoneState)         gCullNoneState         ->Release();
    if (gNoBlendingState)       gNoBlendingState       ->Release();
    if (gAlphaBlendingState)    gAlphaBlendingState    ->Release();
    if (gAdditiveBlendingState) gAdditiveBlendingState ->Release();
    if (gAnisotropic4xSampler)  gAnisotropic4xSampler  ->Release();
    if (gTrilinearSampler)      gTrilinearSampler      ->Release();
    if (gPointSampler)          gPointSampler          ->Release();
}

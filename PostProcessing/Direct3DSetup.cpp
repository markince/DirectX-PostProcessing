//--------------------------------------------------------------------------------------
// Initialisation of Direct3D and main resources (textures, shaders etc.)
//--------------------------------------------------------------------------------------

#include "Direct3DSetup.h"
#include "Shader.h"
#include "Common.h"
#include <d3d11.h>
#include <vector>


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Globals used to keep code simpler, but try to architect your own code in a better way

// The main Direct3D (D3D) variables
ID3D11Device*        gD3DDevice  = nullptr; // D3D device for overall features
ID3D11DeviceContext* gD3DContext = nullptr; // D3D context for specific rendering tasks

// Swap chain and back buffer
IDXGISwapChain*         gSwapChain              = nullptr;
ID3D11RenderTargetView* gBackBufferRenderTarget = nullptr;

// Depth buffer (can also contain "stencil" values, which we will see later)
ID3D11Texture2D*          gDepthStencilTexture = nullptr; // The texture holding the depth values
ID3D11DepthStencilView*   gDepthStencil        = nullptr; // The depth buffer referencing above texture
ID3D11ShaderResourceView* gDepthShaderView     = nullptr; // Allows access to the depth buffer as a texture for certain specialised shaders


//--------------------------------------------------------------------------------------
// Initialise / uninitialise Direct3D
//--------------------------------------------------------------------------------------
// Returns false on failure
bool InitDirect3D()
{
    // Many DirectX functions return a "HRESULT" variable to indicate success or failure. Microsoft code often uses
    // the FAILED macro to test this variable, you'll see it throughout the code - it's fairly self explanatory.
    HRESULT hr = S_OK;


    //// Initialise DirectX ////

    // Create a Direct3D device (i.e. initialise D3D) and create a swap-chain (create a back buffer to render to)
    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.OutputWindow = gHWnd;                           // Target window
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;          // Use DXGI_SWAP_EFFECT_SEQUENTIAL to retain the previous back buffer (for feedback blur effects)
    swapDesc.Windowed = TRUE;
    swapDesc.BufferCount = 1;
    swapDesc.BufferDesc.Width  = gViewportWidth;             // Target window size
    swapDesc.BufferDesc.Height = gViewportHeight;            // --"--
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Pixel format of target window
    swapDesc.BufferDesc.RefreshRate.Numerator   = 60;        // Refresh rate of monitor (provided as fraction = 60/1 here)
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;         // --"--
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.SampleDesc.Count   = 1;
    swapDesc.SampleDesc.Quality = 0;
    UINT flags = D3D11_CREATE_DEVICE_DEBUG; // Set this to 0, or D3D11_CREATE_DEVICE_DEBUG to get more debugging information (in the "Output" window of Visual Studio)
    hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, flags, 0, 0, D3D11_SDK_VERSION,
                                       &swapDesc, &gSwapChain, &gD3DDevice, nullptr, &gD3DContext);
    if (FAILED(hr))
    {
        gLastError = "Error creating Direct3D device";
        return false;
    }


    // Get a "render target view" of back-buffer - standard behaviour
    ID3D11Texture2D* backBuffer;
    hr = gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    if (FAILED(hr))
    {
        gLastError = "Error creating swap chain";
        return false;
    }
    hr = gD3DDevice->CreateRenderTargetView(backBuffer, NULL, &gBackBufferRenderTarget);
    backBuffer->Release();
    if (FAILED(hr))
    {
        gLastError = "Error creating render target view";
        return false;
    }


    //// Create depth buffer to go along with the back buffer ////
    
    // First create a texture to hold the depth buffer values
    D3D11_TEXTURE2D_DESC dbDesc = {};
    dbDesc.Width  = gViewportWidth; // Same size as viewport / back-buffer
    dbDesc.Height = gViewportHeight;
    dbDesc.MipLevels = 1;
    dbDesc.ArraySize = 1;
    dbDesc.Format = DXGI_FORMAT_R32_TYPELESS; // Each depth value is a single float
                                              // Important point for when using depth buffer as texture, must use the TYPELESS constant shown here
    dbDesc.SampleDesc.Count = 1;
    dbDesc.SampleDesc.Quality = 0;
    dbDesc.Usage = D3D11_USAGE_DEFAULT;
    dbDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // Using this depth buffer in shaders so must say so;
    dbDesc.CPUAccessFlags = 0;
    dbDesc.MiscFlags = 0;
    hr = gD3DDevice->CreateTexture2D(&dbDesc, nullptr, &gDepthStencilTexture);
    if (FAILED(hr))
    {
        gLastError = "Error creating depth buffer texture";
        return false;
    }

    // Create the depth stencil view - an object to allow us to use the texture
    // just created as a depth buffer
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT; // Important point for when using depth buffer as texture, ensure you use this setting - different from other labs
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    hr = gD3DDevice->CreateDepthStencilView(gDepthStencilTexture, &dsvDesc, &gDepthStencil);
    if (FAILED(hr))
    {
        gLastError = "Error creating depth buffer view";
        return false;
    }

    // Also create a shader resource view for the depth buffer - required when we want to access the depth buffer as a texture (also note the two important comments in above code)
    // Note the veryt 
    D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
    descSRV.Format = DXGI_FORMAT_R32_FLOAT;
    descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;
    hr = gD3DDevice->CreateShaderResourceView( gDepthStencilTexture, &descSRV, &gDepthShaderView );
    if (FAILED(hr))
    {
        gLastError = "Error creating depth buffer shader resource view";
        return false;
    }
    
    return true;
}


// Release the memory held by all objects created
void ShutdownDirect3D()
{
    // Release each Direct3D object to return resources to the system. Leaving these out will cause memory
    // leaks. Check documentation to see which objects need to be released when adding new features in your
    // own projects.
    if (gD3DContext)
    {
        gD3DContext->ClearState(); // This line is also needed to reset the GPU before shutting down DirectX
        gD3DContext->Release();
    }
    if (gDepthShaderView)        gDepthShaderView->Release();
    if (gDepthStencil)           gDepthStencil->Release();
    if (gDepthStencilTexture)    gDepthStencilTexture->Release();
    if (gBackBufferRenderTarget) gBackBufferRenderTarget->Release();
    if (gSwapChain)              gSwapChain->Release();
    if (gD3DDevice)              gD3DDevice->Release();
}



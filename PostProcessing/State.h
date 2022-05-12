//--------------------------------------------------------------------------------------
// State creation
// - Sampler state (Bilinear, trilinear etc.)
// - Blender state (Additive blending, alpha blending etc.)
// - Rasterizer state (Wireframe mode, don't cull back faces etc.)
// - Depth stencil state (How to use the depth and stencil buffer)
//--------------------------------------------------------------------------------------
#ifndef _STATE_H_INCLUDED_
#define _STATE_H_INCLUDED_

#include <d3d11.h>

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Make global variables available to other files. "extern" means this variable is defined in another
// file somewhere. We should use classes and avoid use of globals, but done this way to keep code simpler
// so the DirectX content is clearer. However, try to architect your own code in a better way.

// GPU "States" //
extern ID3D11SamplerState* gPointSampler;
extern ID3D11SamplerState* gTrilinearSampler;
extern ID3D11SamplerState* gAnisotropic4xSampler;

extern ID3D11BlendState* gNoBlendingState;
extern ID3D11BlendState* gAdditiveBlendingState;
extern ID3D11BlendState* gAlphaBlendingState;

extern ID3D11RasterizerState*   gCullBackState;
extern ID3D11RasterizerState*   gCullFrontState;
extern ID3D11RasterizerState*   gCullNoneState;

extern ID3D11DepthStencilState* gUseDepthBufferState;
extern ID3D11DepthStencilState* gDepthReadOnlyState;
extern ID3D11DepthStencilState* gNoDepthBufferState;


//--------------------------------------------------------------------------------------
// State creation / destruction
//--------------------------------------------------------------------------------------

// Create all the states used in this app, returns true on success
bool CreateStates();

// Release DirectX state objects
void ReleaseStates();


#endif //_STATE_H_INCLUDED_

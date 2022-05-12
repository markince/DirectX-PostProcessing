//--------------------------------------------------------------------------------------
// Commonly used definitions across entire project
//--------------------------------------------------------------------------------------
#ifndef _COMMON_H_INCLUDED_
#define _COMMON_H_INCLUDED_

#include "CVector2.h"
#include "CVector3.h"
#include "CVector4.h"
#include "CMatrix4x4.h"

#include <d3d11.h>
#include <string>


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Make global Variables from various files available to other files. "extern" means
// this variable is defined in another file somewhere. We should use classes and avoid
// use of globals, but done this way to keep code simpler so the DirectX content is
// clearer. However, try to architect your own code in a better way.

// Windows variables
extern HWND gHWnd;

// Viewport size
extern int gViewportWidth;
extern int gViewportHeight;


// Important DirectX variables
extern ID3D11Device*             gD3DDevice;
extern ID3D11DeviceContext*      gD3DContext;

extern IDXGISwapChain*           gSwapChain;
extern ID3D11RenderTargetView*   gBackBufferRenderTarget; // Back buffer is where we render to
extern ID3D11DepthStencilView*   gDepthStencil;           // The depth buffer contains a depth for each back buffer pixel
extern ID3D11ShaderResourceView* gDepthShaderView;        // Allows access to the depth buffer as a texture for certain specialised shaders
extern ID3D11VertexShader*       g2DPolygonVertexShader;

// Input constsnts
extern const float ROTATION_SPEED;
extern const float MOVEMENT_SPEED;


// A global error message to help track down fatal errors - set it to a useful message
// when a serious error occurs
extern std::string gLastError;

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame

// Data that remains constant for an entire frame, updated from C++ to the GPU shaders *once per frame*
// We hold them together in a structure and send the whole thing to a "constant buffer" on the GPU each frame when
// we have finished updating the scene. There is a structure in the shader code that exactly matches this one
struct PerFrameConstants
{
    // These are the matrices used to position the camera
	CMatrix4x4 cameraMatrix;
	CMatrix4x4 viewMatrix;
	CMatrix4x4 projectionMatrix;
	CMatrix4x4 viewProjectionMatrix; // The above two matrices multiplied together to combine their effects

	CVector3   light1Position; // 3 floats: x, y z
	float      viewportWidth;  // Using viewport width and height as padding - see this structure in earlier labs to read about padding here
	CVector3   light1Colour;
	float      viewportHeight;

    CVector3   light2Position;
    float      padding1;
    CVector3   light2Colour;
    float      padding2;

    CVector3   ambientColour;
    float      specularPower;

    CVector3   cameraPosition;
	float      frameTime;      // This app does updates on the GPU so we pass over the frame update time
};

extern PerFrameConstants gPerFrameConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*     gPerFrameConstantBuffer; // This variable controls the GPU-side constant buffer matching to the above structure



static const int MAX_BONES = 64;

// This is the matrix that positions the next thing to be rendered in the scene. Unlike the structure above this data can be
// updated and sent to the GPU several times every frame (once per model). However, apart from that it works in the same way.
struct PerModelConstants
{
    CMatrix4x4 worldMatrix;

    CVector3   objectColour;  // Allows each light model to be tinted to match the light colour they cast
	float      explodeAmount; // Used in the geometry shader to control how much the polygons are exploded outwards

	CMatrix4x4 boneMatrices[MAX_BONES];
};
extern PerModelConstants gPerModelConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*     gPerModelConstantBuffer; // This variable controls the GPU-side constant buffer related to the above structure




//**************************

// Settings used by post-processes - must match the similar structure in the Common.hlsli shader file
struct PostProcessingConstants
{
	CVector2 area2DTopLeft; // Top-left of post-process area on screen, provided as coordinate from 0.0->1.0 not as a pixel coordinate
	CVector2 area2DSize;    // Size of post-process area on screen, provided as sizes from 0.0->1.0 (1 = full screen) not as a size in pixels
	float    area2DDepth;   // Depth buffer value for area (0.0 nearest to 1.0 furthest). Full screen post-processing uses 0.0f
	CVector3 paddingA;      // Pad things to collections of 4 floats (see notes in earlier labs to read about padding)

	CVector4 polygon2DPoints[4]; // Four points of a polygon in 2D viewport space for polygon post-processing. Matrix transformations already done on C++ side

	// Tint post-process settings
	CVector3 tintColour;
	float    paddingB;  // Pad things to collections of 4 floats (see notes in earlier labs to read about padding)

	// Grey noise post-process settings
    CVector2 noiseScale;
	CVector2 noiseOffset;

	// Burn post-process settings
	float    burnHeight;
	CVector3 paddingC;

	// Distort post-process settings
	float    distortLevel;
	CVector3 paddingD;

	// Spiral post-process settings
	float    spiralLevel;
	CVector3 paddingE;

	// Under Water post-process settings
	float underWaterLevel;
	CVector3 paddingF;

	// hue shift
	float hueShift;
	CVector3 paddingG;

	CVector2 blurDirection;
	CVector3 paddingH;

	bool feedbackBlur;

};
extern PostProcessingConstants gPostProcessingConstants;      // This variable holds the CPU-side constant buffer described above
extern ID3D11Buffer*           gPostProcessingConstantBuffer; // This variable controls the GPU-side constant buffer related to the above structure

//**************************


#endif //_COMMON_H_INCLUDED_

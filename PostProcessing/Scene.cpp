//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"

#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"
#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here
#include "ColourRGBA.h" 

#include <sstream>
#include <memory>
#include <array>


//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------

//********************
// Available post-processes
enum class PostProcess
{
	None,
	VColourGradient,
	HLSGradient,
	FullScreenBlur,
	GaussianBlur,
	UnderWater,
	Retro,
	Bloom,

	Burn,
	Distort,
	GreyNoise,
	Spiral,
	Tint,
};

// Current active post proccesss
auto gCurrentPostProcess = PostProcess::None;


// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 1.5f;  // Radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // Units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)

// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
bool lockFPS = true;

// Meshes, models and cameras, same meaning as TL-Engine. Meshes prepared in InitGeometry function, Models & camera in InitScene
Mesh* gStarsMesh;
Mesh* gGroundMesh;
Mesh* gCubeMesh;
Mesh* gCrateMesh;
Mesh* gTrollMesh;
Mesh* gLightMesh;
Mesh* gTeapotMesh;
Mesh* gWall1Mesh;
Mesh* gWall2Mesh;


Model* gStars;
Model* gGround;
Model* gCube;
Model* gCrate;
Model* gTroll;
Model* gTeapot;
Model* gWall1;
Model* gWall2;

Camera* gCamera;


// Store lights in an array in this exercise
const int NUM_LIGHTS = 2;
struct Light
{
	Model* model;
	CVector3 colour;
	float    strength;
};
Light gLights[NUM_LIGHTS];


// Additional light information
CVector3 gAmbientColour = { 0.3f, 0.3f, 0.4f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
float    gSpecularPower = 256; // Specular power controls shininess - same for all models in this app

ColourRGBA gBackgroundColor = { 0.3f, 0.3f, 0.4f, 1.0f };

// Variables controlling light1's orbiting of the cube
const float gLightOrbitRadius = 20.0f;
const float gLightOrbitSpeed = 0.7f;

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame
// The structures are now in Common.h
// IMPORTANT: Any new data you add in C++ code (CPU-side) is not automatically available to the GPU
//            Anything the shaders need (per-frame or per-model) needs to be sent via a constant buffer

PerFrameConstants gPerFrameConstants;      // The constants (settings) that need to be sent to the GPU each frame (see common.h for structure)
ID3D11Buffer*     gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constants (settings) that change per-model (e.g. world matrix)
ID3D11Buffer*     gPerModelConstantBuffer; // --"--

//**************************
PostProcessingConstants gPostProcessingConstants;       // As above, but constants (settings) for each post-process
ID3D11Buffer*           gPostProcessingConstantBuffer; // --"--
//**************************


//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

// DirectX objects controlling textures used in this lab
ID3D11Resource*           gStarsDiffuseSpecularMap     = nullptr;
ID3D11ShaderResourceView* gStarsDiffuseSpecularMapSRV  = nullptr;
ID3D11Resource*           gGroundDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gGroundDiffuseSpecularMapSRV = nullptr;
ID3D11Resource*           gCrateDiffuseSpecularMap     = nullptr;
ID3D11ShaderResourceView* gCrateDiffuseSpecularMapSRV  = nullptr;
ID3D11Resource*           gTrollDiffuseSpecularMap     = nullptr;
ID3D11ShaderResourceView* gTrollDiffuseSpecularMapSRV  = nullptr;
ID3D11Resource*           gTeapotDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gTeapotDiffuseSpecularMapSRV = nullptr;
ID3D11Resource*           gCubeDiffuseSpecularMap      = nullptr;
ID3D11ShaderResourceView* gCubeDiffuseSpecularMapSRV   = nullptr;
ID3D11Resource*           gWall1DiffuseSpecularMap     = nullptr;
ID3D11ShaderResourceView* gWall1DiffuseSpecularMapSRV  = nullptr;
ID3D11Resource*           gWall2DiffuseSpecularMap     = nullptr;
ID3D11ShaderResourceView* gWall2DiffuseSpecularMapSRV  = nullptr;

ID3D11Resource*           gLightDiffuseMap             = nullptr;
ID3D11ShaderResourceView* gLightDiffuseMapSRV          = nullptr;


//****************************
// Post processing textures

// This texture will have the scene renderered on it. Then the texture is then used for post-processing
ID3D11Texture2D*          gSceneTexture1      = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11RenderTargetView*   gSceneRenderTarget1 = nullptr; // This object is used when we want to render to the texture above
ID3D11ShaderResourceView* gSceneTextureSRV1   = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

//// This texture will have the scene renderered on it. Then the texture is then used for post-processing
ID3D11Texture2D*          gSceneTexture2      = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11RenderTargetView*   gSceneRenderTarget2 = nullptr; // This object is used when we want to render to the texture above
ID3D11ShaderResourceView* gSceneTextureSRV2   = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

// Additional textures used for specific post-processes
ID3D11Resource*           gNoiseMap      = nullptr;
ID3D11ShaderResourceView* gNoiseMapSRV   = nullptr;
ID3D11Resource*           gBurnMap       = nullptr;
ID3D11ShaderResourceView* gBurnMapSRV    = nullptr;
ID3D11Resource*           gDistortMap    = nullptr;
ID3D11ShaderResourceView* gDistortMapSRV = nullptr;


//****************************
// Functions used in poloygon post processing

CVector4 TransformVector4(const CVector4& v, const CMatrix4x4& m)
{
	CVector4 vOut;

	vOut.x = v.x * m.e00 + v.y * m.e10 + v.z * m.e20 + v.w * m.e30;
	vOut.y = v.x * m.e01 + v.y * m.e11 + v.z * m.e21 + v.w * m.e31;
	vOut.z = v.x * m.e02 + v.y * m.e12 + v.z * m.e22 + v.w * m.e32;
	vOut.w = v.x * m.e03 + v.y * m.e13 + v.z * m.e23 + v.w * m.e33;

	return vOut;
}

// Effects list used for stacking effects
std::vector<PostProcess> postProcessEffectList;

// Current target used for stacking effects
ID3D11RenderTargetView*   gCurrentTarget;
ID3D11ShaderResourceView* gCurrentSRV;

// Square window opening
const std::array<CVector3, 4> PolygonPoints =
{
	{
		{ -1, 1, 0 },  // TL
		{  1, 1, 0 },  // TR
		{ -1,-1, 0 },  // BL
		{  1,-1, 0 }   // BR
	}
};

// This line  is used to scale the polygon and place it in correct position
CMatrix4x4 SquareWindowPolygonMatrix   = MatrixScaling({ 5, 5, 5 }) * MatrixTranslation({ 0, 10, -50 });
CMatrix4x4 SpadeWindowPolygonMatrix    = MatrixScaling({ 5, 5, 5 }) * MatrixRotationY(ToRadians(90.0f)) * MatrixTranslation({ -60, 10,  18 });
CMatrix4x4 DiamondWindowPolygonMatrix  = MatrixScaling({ 5, 5, 5 }) * MatrixRotationY(ToRadians(90.0f)) * MatrixTranslation({ -60, 10,   5 });
CMatrix4x4 ClubWindowPolygonMatrix     = MatrixScaling({ 5, 5, 5 }) * MatrixRotationY(ToRadians(90.0f)) * MatrixTranslation({ -60, 10,  -6 });
CMatrix4x4 HeartWindowPolygonMatrix    = MatrixScaling({ 5, 5, 5 }) * MatrixRotationY(ToRadians(90.0f)) * MatrixTranslation({ -60, 10, -19 });



// Function to add a process to the list (used in process stacking effects)
void UpdatePostProcessEffectsList(PostProcess postProcessEffectType)
{
	if (postProcessEffectType != PostProcess::None)
	{
		postProcessEffectList.push_back(postProcessEffectType);

	}
}

// Function to reset the list (used in process stacking effects)
void ResetPostProcessEffectsList()
{
	postProcessEffectList.clear();
}

// Function to swap post processing effects (used in the stacking process)

void SwapPostProcessEffect()
{
	if      (gCurrentTarget == gSceneRenderTarget1) gCurrentTarget = gSceneRenderTarget2;
	else if (gCurrentTarget == gSceneRenderTarget2) gCurrentTarget = gSceneRenderTarget1;

	if      (gCurrentSRV == gSceneTextureSRV1) gCurrentSRV = gSceneTextureSRV2;
	else if (gCurrentSRV == gSceneTextureSRV2) gCurrentSRV = gSceneTextureSRV1;
}


//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry()
{
	////--------------- Load meshes ---------------////

	// Load mesh geometry data, just like TL-Engine this doesn't create anything in the scene. Create a Model for that.
	try
	{
		gStarsMesh  = new Mesh("Stars.x");
		gGroundMesh = new Mesh("Ground.x");
		gCubeMesh   = new Mesh("Cube.x");
		gCrateMesh  = new Mesh("CargoContainer.x");
		gTrollMesh  = new Mesh("Troll.x");
		gTeapotMesh = new Mesh("Teapot.x");
		gLightMesh  = new Mesh("Light.x");
		gWall1Mesh  = new Mesh("Wall1.x");
		gWall2Mesh  = new Mesh("Wall2.x");
	}
	catch (std::runtime_error e)  // Constructors cannot return error messages so use exceptions to catch mesh errors (fairly standard approach this)
	{
		gLastError = e.what(); // This picks up the error message put in the exception (see Mesh.cpp)
		return false;
	}

	////--------------- Load / prepare textures & GPU states ---------------////

	// Load textures and create DirectX objects for them
	// The LoadTexture function requires you to pass a ID3D11Resource* (e.g. &gCubeDiffuseMap), which manages the GPU memory for the
	// texture and also a ID3D11ShaderResourceView* (e.g. &gCubeDiffuseMapSRV), which allows us to use the texture in shaders
	// The function will fill in these pointers with usable data. The variables used here are globals found near the top of the file.
	if (!LoadTexture("Stars.jpg",                &gStarsDiffuseSpecularMap,  &gStarsDiffuseSpecularMapSRV)  ||
		!LoadTexture("GrassDiffuseSpecular.dds", &gGroundDiffuseSpecularMap, &gGroundDiffuseSpecularMapSRV) ||
		!LoadTexture("StoneDiffuseSpecular.dds", &gCubeDiffuseSpecularMap,   &gCubeDiffuseSpecularMapSRV)   ||
		!LoadTexture("CargoA.dds",               &gCrateDiffuseSpecularMap,  &gCrateDiffuseSpecularMapSRV)  ||
		!LoadTexture("TrollDiffuseSpecular.dds", &gTrollDiffuseSpecularMap,  &gTrollDiffuseSpecularMapSRV)  ||
		!LoadTexture("tiles1.jpg",               &gTeapotDiffuseSpecularMap, &gTeapotDiffuseSpecularMapSRV) ||
		!LoadTexture("Flare.jpg",                &gLightDiffuseMap,          &gLightDiffuseMapSRV)          ||
		!LoadTexture("Noise.png",                &gNoiseMap,                 &gNoiseMapSRV)                 ||
		!LoadTexture("Burn.png",                 &gBurnMap,                  &gBurnMapSRV)                  ||
		!LoadTexture("Distort.png",              &gDistortMap,               &gDistortMapSRV)               ||
		!LoadTexture("brick_35.jpg",             &gWall1DiffuseSpecularMap,  &gWall1DiffuseSpecularMapSRV))
	{
		gLastError = "Error loading textures";
		return false;
	}


	// Create all filtering modes, blending modes etc. used by the app (see State.cpp/.h)
	if (!CreateStates())
	{
		gLastError = "Error creating states";
		return false;
	}


	////--------------- Prepare shaders and constant buffers to communicate with them ---------------////

	// Load the shaders required for the geometry we will use (see Shader.cpp / .h)
	if (!LoadShaders())
	{
		gLastError = "Error loading shaders";
		return false;
	}

	// Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
	// These allow us to pass data from CPU to shaders such as lighting information or matrices
	// See the comments above where these variable are declared and also the UpdateScene function
	gPerFrameConstantBuffer       = CreateConstantBuffer(sizeof(gPerFrameConstants));
	gPerModelConstantBuffer       = CreateConstantBuffer(sizeof(gPerModelConstants));
	gPostProcessingConstantBuffer = CreateConstantBuffer(sizeof(gPostProcessingConstants));
	if (gPerFrameConstantBuffer == nullptr || gPerModelConstantBuffer == nullptr || gPostProcessingConstantBuffer == nullptr)
	{
		gLastError = "Error creating constant buffers";
		return false;
	}

	//********************************************
	//**** Create Scene Texture

	// We will render the scene to this texture instead of the back-buffer (screen), then we post-process the texture onto the screen
	// This is exactly the same code we used in the graphics module when we were rendering the scene onto a cube using a texture

	// Using a helper function to load textures from files above. Here we create the scene texture manually
	// as we are creating a special kind of texture (one that we can render to). Many settings to prepare:
	D3D11_TEXTURE2D_DESC sceneTextureDesc = {};
	sceneTextureDesc.Width = gViewportWidth;  // Full-screen post-processing - use full screen size for texture
	sceneTextureDesc.Height = gViewportHeight;
	sceneTextureDesc.MipLevels = 1; // No mip-maps when rendering to textures (or we would have to render every level)
	sceneTextureDesc.ArraySize = 1;
	sceneTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA texture (8-bits each)
	sceneTextureDesc.SampleDesc.Count = 1;
	sceneTextureDesc.SampleDesc.Quality = 0;
	sceneTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	sceneTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // IMPORTANT: Indicate we will use texture as render target, and pass it to shaders
	sceneTextureDesc.CPUAccessFlags = 0;
	sceneTextureDesc.MiscFlags = 0;
	if (FAILED(gD3DDevice->CreateTexture2D(&sceneTextureDesc, NULL, &gSceneTexture1)))
	{
		gLastError = "Error creating scene texture";
		return false;
	}
	if (FAILED(gD3DDevice->CreateTexture2D(&sceneTextureDesc, NULL, &gSceneTexture2)))
	{
		gLastError = "Error creating scene texture";
		return false;
	}

	// We created the scene texture above, now we get a "view" of it as a render target, i.e. get a special pointer to the texture that
	// we use when rendering to it (see RenderScene function below)
	if (FAILED(gD3DDevice->CreateRenderTargetView(gSceneTexture1, NULL, &gSceneRenderTarget1)))
	{
		gLastError = "Error creating scene render target view";
		return false;
	}
	if (FAILED(gD3DDevice->CreateRenderTargetView(gSceneTexture2, NULL, &gSceneRenderTarget2)))
	{
		gLastError = "Error creating scene render target view";
		return false;
	}

	// We also need to send this texture (resource) to the shaders. To do that we must create a shader-resource "view"
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc = {};
	srDesc.Format = sceneTextureDesc.Format;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	if (FAILED(gD3DDevice->CreateShaderResourceView(gSceneTexture1, &srDesc, &gSceneTextureSRV1)))
	{
		gLastError = "Error creating scene shader resource view";
		return false;
	}
	if (FAILED(gD3DDevice->CreateShaderResourceView(gSceneTexture2, &srDesc, &gSceneTextureSRV2)))
	{
		gLastError = "Error creating scene shader resource view";
		return false;
	}

	return true;
}


// Prepare the scene
// Returns true on success
bool InitScene()
{
	////--------------- Set up scene ---------------////

	// Models

	gStars  = new Model(gStarsMesh);
	gGround = new Model(gGroundMesh);
	gCube   = new Model(gCubeMesh);
	gCrate  = new Model(gCrateMesh);
	gTroll  = new Model(gTrollMesh);
	gTeapot = new Model(gTeapotMesh);
	gWall1  = new Model(gWall1Mesh);
	gWall2  = new Model(gWall2Mesh);

	// Positions
	gCube   ->SetPosition( {  42.0f, 5.0f, -10.0f } );
	gCrate  ->SetPosition( { -10.0f, 0.0f,  90.0f } );
	gTroll  ->SetPosition( { -30.0f, 0.0f, -20.0f } );
	gTeapot ->SetPosition( { -20.0f, 0.0f,  20.0f } );
	gWall1  ->SetPosition( {   0.0f, 0.0f, -50.0f } );
	gWall2  ->SetPosition( { -60.0f, 0.0f,   0.0f } );


	// Rotations
	gCube   ->SetRotation( { 0.0f, ToRadians( -110.0f), 0.0f } );
	gCrate  ->SetRotation( { 0.0f, ToRadians(   40.0f), 0.0f } );
	gTroll  ->SetRotation( { 0.0f, ToRadians(  200.0f), 0.0f } );
	gTeapot ->SetRotation( { 0.0f, ToRadians(    0.0f), 0.0f } );
	gWall1  ->SetRotation( { 0.0f, ToRadians(    0.0f), 0.0f } );
	gWall2  ->SetRotation( { 0.0f, ToRadians(   90.0f), 0.0f } );


	// Scaling
	gCube   ->SetScale(    1.5f );
	gCrate  ->SetScale(    6.0f );
	gStars  ->SetScale( 8000.0f );
	gTroll  ->SetScale(   10.0f );
	gTeapot ->SetScale(    1.0f );
	gWall1  ->SetScale(   40.0f );
	gWall2  ->SetScale(   40.0f );


	// Light set-up - using an array this time
	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		gLights[i].model = new Model(gLightMesh);
	}

	gLights[0].colour = { 0.8f, 0.8f, 1.0f };
	gLights[0].strength = 10;
	gLights[0].model->SetPosition({ 30, 10, 0 });
	gLights[0].model->SetScale(pow(gLights[0].strength, 0.7f)); // Convert light strength into a nice value for the scale of the light - equation is ad-hoc.

	gLights[1].colour = { 1.0f, 0.8f, 0.2f };
	gLights[1].strength = 40;
	gLights[1].model->SetPosition({ -70, 30, 100 });
	gLights[1].model->SetScale(pow(gLights[1].strength, 0.7f));

	////--------------- Set up camera ---------------////

	gCamera = new Camera();
	gCamera->SetPosition({ -100, 80, -100 });
	gCamera->SetRotation({ ToRadians(30.0f), ToRadians(40.0f), 0.0f });

	return true;
}


// Release the geometry and scene resources created above
void ReleaseResources()
{
	ReleaseStates();

	if (gSceneTextureSRV1)   gSceneTextureSRV1   ->Release();
	if (gSceneRenderTarget1) gSceneRenderTarget1 ->Release();
	if (gSceneTexture1)      gSceneTexture1      ->Release();
	if (gSceneTextureSRV2)   gSceneTextureSRV2   ->Release();
	if (gSceneRenderTarget2) gSceneRenderTarget2 ->Release();
	if (gSceneTexture2)      gSceneTexture2      ->Release();

	if (gDistortMapSRV) gDistortMapSRV ->Release();
	if (gDistortMap)    gDistortMap    ->Release();
	if (gBurnMapSRV)    gBurnMapSRV    ->Release();
	if (gBurnMap)       gBurnMap       ->Release();
	if (gNoiseMapSRV)   gNoiseMapSRV   ->Release();
	if (gNoiseMap)      gNoiseMap      ->Release();

	if (gLightDiffuseMapSRV)          gLightDiffuseMapSRV          ->Release();
	if (gLightDiffuseMap)             gLightDiffuseMap             ->Release();
	if (gCrateDiffuseSpecularMapSRV)  gCrateDiffuseSpecularMapSRV  ->Release();
	if (gCrateDiffuseSpecularMap)     gCrateDiffuseSpecularMap     ->Release();
	if (gCubeDiffuseSpecularMapSRV)   gCubeDiffuseSpecularMapSRV   ->Release();
	if (gCubeDiffuseSpecularMap)      gCubeDiffuseSpecularMap      ->Release();
	if (gGroundDiffuseSpecularMapSRV) gGroundDiffuseSpecularMapSRV ->Release();
	if (gGroundDiffuseSpecularMap)    gGroundDiffuseSpecularMap    ->Release();
	if (gStarsDiffuseSpecularMapSRV)  gStarsDiffuseSpecularMapSRV  ->Release();
	if (gStarsDiffuseSpecularMap)     gStarsDiffuseSpecularMap     ->Release();
	if (gTrollDiffuseSpecularMap)     gTrollDiffuseSpecularMap     ->Release();
	if (gTrollDiffuseSpecularMapSRV)  gTrollDiffuseSpecularMapSRV  ->Release();
	if (gTeapotDiffuseSpecularMap)    gTeapotDiffuseSpecularMap    ->Release();
	if (gTeapotDiffuseSpecularMapSRV) gTeapotDiffuseSpecularMapSRV ->Release();
	if (gWall1DiffuseSpecularMap)     gWall1DiffuseSpecularMap     ->Release();
	if (gWall1DiffuseSpecularMapSRV)  gWall1DiffuseSpecularMapSRV  ->Release();

	if (gPostProcessingConstantBuffer)  gPostProcessingConstantBuffer ->Release();
	if (gPerModelConstantBuffer)        gPerModelConstantBuffer       ->Release();
	if (gPerFrameConstantBuffer)        gPerFrameConstantBuffer       ->Release();

	ReleaseShaders();

	// See note in InitGeometry about why we're not using unique_ptr and having to manually delete
	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		delete gLights[i].model;  gLights[i].model = nullptr;
	}
	delete gCamera; gCamera = nullptr;
	delete gCrate;  gCrate  = nullptr;
	delete gCube;   gCube   = nullptr;
	delete gGround; gGround = nullptr;
	delete gStars;  gStars  = nullptr;
	delete gTroll;  gTroll  = nullptr;
	delete gTeapot; gTeapot = nullptr;
	delete gWall1;  gWall1  = nullptr;
	delete gWall2;  gWall2  = nullptr;


	delete gLightMesh;  gLightMesh  = nullptr;
	delete gCrateMesh;  gCrateMesh  = nullptr;
	delete gCubeMesh;   gCubeMesh   = nullptr;
	delete gGroundMesh; gGroundMesh = nullptr;
	delete gStarsMesh;  gStarsMesh  = nullptr;
	delete gWall1Mesh;  gWall1Mesh  = nullptr;
	delete gWall2Mesh;  gWall2Mesh = nullptr;
}



//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------

// Render everything in the scene from the given camera
void RenderSceneFromCamera(Camera* camera)
{
	// Set camera matrices in the constant buffer and send over to GPU
	gPerFrameConstants.cameraMatrix = camera->WorldMatrix();
	gPerFrameConstants.viewMatrix = camera->ViewMatrix();
	gPerFrameConstants.projectionMatrix = camera->ProjectionMatrix();
	gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
	UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

	// Indicate that the constant buffer we just updated is for use in the vertex shader (VS), geometry shader (GS) and pixel shader (PS)
	gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
	gD3DContext->GSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);
	gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);

	gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);

	////--------------- Render ordinary models ---------------///

	// Select which shaders to use next
	gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gPixelLightingPixelShader, nullptr, 0);
	gD3DContext->GSSetShader(nullptr, nullptr, 0);  // Switch off geometry shader when not using it (pass nullptr for first parameter)

	// States - no blending, normal depth buffer and back-face culling (standard set-up for opaque models)
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
	gD3DContext->RSSetState(gCullBackState);

	// Render lit models, only change textures for each onee
	gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

	gD3DContext->PSSetShaderResources(0, 1, &gGroundDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gGround->Render();

	gD3DContext->PSSetShaderResources(0, 1, &gCrateDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gCrate->Render();

	gD3DContext->PSSetShaderResources(0, 1, &gCubeDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gCube->Render();

	gD3DContext->PSSetShaderResources(0, 1, &gTrollDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gTroll->Render();

	gD3DContext->PSSetShaderResources(0, 1, &gTeapotDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gTeapot->Render();

	gD3DContext->PSSetShaderResources(0, 1, &gWall1DiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
	gWall1->Render();
	gWall2->Render();

	////--------------- Render sky ---------------////

	// Select which shaders to use next
	gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gTintedTexturePixelShader, nullptr, 0);

	// Using a pixel shader that tints the texture - don't need a tint on the sky so set it to white
	gPerModelConstants.objectColour = { 1, 1, 1 };

	// Stars point inwards
	gD3DContext->RSSetState(gCullNoneState);

	// Render sky
	gD3DContext->PSSetShaderResources(0, 1, &gStarsDiffuseSpecularMapSRV);
	gStars->Render();

	////--------------- Render lights ---------------////

	// Select which shaders to use next (actually same as before, so we could skip this)
	gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
	gD3DContext->PSSetShader(gTintedTexturePixelShader, nullptr, 0);

	// Select the texture and sampler to use in the pixel shader
	gD3DContext->PSSetShaderResources(0, 1, &gLightDiffuseMapSRV); // First parameter must match texture slot number in the shaer

	// States - additive blending, read-only depth buffer and no culling (standard set-up for blending)
	gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
	gD3DContext->RSSetState(gCullNoneState);

	// Render all the lights in the array
	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		gPerModelConstants.objectColour = gLights[i].colour; // Set any per-model constants apart from the world matrix just before calling render (light colour here)
		gLights[i].model->Render();
	}
}

//**************************
// Run any scene post-processing steps

void PostProcessing(ID3D11RenderTargetView* RenderTarget, ID3D11ShaderResourceView* NextSRV, float frameTime)
{
	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &RenderTarget, gDepthStencil);

	// Give the pixel shader (post-processing shader) access to the scene texture 
	gD3DContext->PSSetShaderResources(0, 1, &NextSRV);
	gD3DContext->PSSetSamplers(0, 1, &gPointSampler); // Use point sampling (no bilinear, trilinear, mip-mapping etc. for most post-processes)

	// Using special vertex shader than creates its own data for a full screen quad
	gD3DContext->VSSetShader(gFullScreenQuadVertexShader, nullptr, 0);
	gD3DContext->GSSetShader(nullptr, nullptr, 0);  // Switch off geometry shader when not using it (pass nullptr for first parameter)

	// States - no blending, ignore depth buffer and culling
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
	gD3DContext->RSSetState(gCullNoneState);

	// No need to set vertex/index buffer (see fullscreen quad vertex shader), just indicate that the quad will be created as a triangle strip
	gD3DContext->IASetInputLayout(NULL); // No vertex data
	gD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Prepare custom settings for current post-process
	if (gCurrentPostProcess == PostProcess::None)
	{
		gD3DContext->PSSetShader(gCopyPixelShader, nullptr, 0);
	}
	else if (gCurrentPostProcess == PostProcess::Tint)
	{
		gD3DContext->PSSetShader(gTintPostProcess, nullptr, 0);
		gPostProcessingConstants.tintColour = { 1, 0, 0 };
	}
	else if (gCurrentPostProcess == PostProcess::GreyNoise)
	{
		gD3DContext->PSSetShader(gGreyNoisePostProcess, nullptr, 0);

		// Noise scaling adjusts how fine the noise is.
		const float grainSize = 50; // Fineness of the noise grain
		gPostProcessingConstants.noiseScale = { gViewportWidth / grainSize, gViewportHeight / grainSize };

		// The noise offset is randomised to give a constantly changing noise effect (like tv static)
		gPostProcessingConstants.noiseOffset = { Random(0.0f, 1.0f), Random(0.0f, 1.0f) };

		// Give pixel shader access to the noise texture
		gD3DContext->PSSetShaderResources(1, 1, &gNoiseMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
	}
	else if (gCurrentPostProcess == PostProcess::Burn)
	{
		gD3DContext->PSSetShader(gBurnPostProcess, nullptr, 0);

		// Set and increase the burn level (cycling back to 0 when it reaches 1.0f)
		const float burnSpeed = 0.2f;
		gPostProcessingConstants.burnHeight = fmod(gPostProcessingConstants.burnHeight + burnSpeed * frameTime, 1.0f);

		// Give pixel shader access to the burn texture (basically a height map that the burn level ascends)
		gD3DContext->PSSetShaderResources(1, 1, &gBurnMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
	}
	else if (gCurrentPostProcess == PostProcess::Distort)
	{
		gD3DContext->PSSetShader(gDistortPostProcess, nullptr, 0);

		// Set the level of distortion
		gPostProcessingConstants.distortLevel = 0.03f;

		// Give pixel shader access to the distortion texture (containts 2D vectors (in R & G) to shift the texture UVs to give a cut-glass impression)
		gD3DContext->PSSetShaderResources(1, 1, &gDistortMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
	}
	else if (gCurrentPostProcess == PostProcess::Spiral)
	{
		gD3DContext->PSSetShader(gSpiralPostProcess, nullptr, 0);

		static float wiggle = 0.0f;
		const float wiggleSpeed = 1.0f;

		// Set and increase the amount of spiral, uses a tweaked cos wave to animate
		gPostProcessingConstants.spiralLevel = ((1.0f - cos(wiggle)) * 4.0f);
		wiggle += wiggleSpeed * frameTime;
	}
	else if (gCurrentPostProcess == PostProcess::VColourGradient)
	{
		gD3DContext->PSSetShader(gVColourGradientPostProcess, nullptr, 0);
	}
	else if (gCurrentPostProcess == PostProcess::HLSGradient)
	{
		gD3DContext->PSSetShader(gHLSGradientPostProcess, nullptr, 0);

		static float wiggle = 0.0f;
		const float wiggleSpeed = 0.5f;
		gPostProcessingConstants.hueShift = wiggle;
		wiggle += wiggleSpeed * frameTime;
	}
	else if (gCurrentPostProcess == PostProcess::FullScreenBlur)
	{
		gD3DContext->PSSetShader(gFullScreenBlurPostProcess, nullptr, 0);
	}
	else if (gCurrentPostProcess == PostProcess::UnderWater)
	{
		gD3DContext->PSSetShader(gUnderWaterPostProcess, nullptr, 0);

		static float waterWiggle = 0.0f;
		const float waterWiggleSpeed = 0.5f;

		gPostProcessingConstants.underWaterLevel = waterWiggle;
		waterWiggle = waterWiggle + waterWiggleSpeed * frameTime;
	}
	else if (gCurrentPostProcess == PostProcess::Retro)
	{
		gD3DContext->PSSetShader(gRetroPostProcess, nullptr, 0);
	}
	else if (gCurrentPostProcess == PostProcess::GaussianBlur)
	{
		gD3DContext->PSSetShader(gGaussianBlurPostProcess, nullptr, 0);
	}
	else if (gCurrentPostProcess == PostProcess::Bloom)
	{
		gD3DContext->PSSetShader(gBloomPostProcess, nullptr, 0);
	}

	// Set 2D area for full-screen post-processing (coordinates in 0->1 range)
	gPostProcessingConstants.area2DTopLeft = { 0, 0 }; // Top-left of entire screen
	gPostProcessingConstants.area2DSize    = { 1, 1 }; // Full size of screen
	gPostProcessingConstants.area2DDepth   = 0;        // Depth buffer value for full screen is as close as possible

	// Pass over the above post-processing settings (also the per-process settings prepared in UpdateScene function below)
	UpdateConstantBuffer(gPostProcessingConstantBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);

	// Draw a quad
	gD3DContext->Draw(4, 0);

	// These lines unbind the scene texture from the pixel shader to stop DirectX issuing a warning when we try to render to it again next frame
	ID3D11ShaderResourceView* nullSRV = nullptr;
	gD3DContext->PSSetShaderResources(0, 1, &nullSRV);
}

//**************************
// Perform an post process from "scene texture" to back buffer within the given four-point polygon and a world matrix to position/rotate/scale the polygon

void PolygonPostProcess(ID3D11PixelShader* postProcess, std::array<CVector3, 4> points, CMatrix4x4 worldMatrix, float frameTime)
{
	// Select the back buffer to use for rendering. Not going to clear the back-buffer because we're going to overwrite it all
	gD3DContext->OMSetRenderTargets(1, &gCurrentTarget, gDepthStencil);

	// Give the pixel shader (post-processing shader) access to the scene texture 
	gD3DContext->PSSetShaderResources(0, 1, &gCurrentSRV);
	gD3DContext->PSSetSamplers(0, 1, &gPointSampler); // Use point sampling (no bilinear, trilinear, mip-mapping etc. for most post-processes)

	// Using special vertex shader than creates its own data for a full screen quad
	gD3DContext->VSSetShader(g2DPolygonVertexShader, nullptr, 0);
	gD3DContext->GSSetShader(nullptr, nullptr, 0);

	// States - no blending, ignore depth buffer and culling
	gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
	gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
	gD3DContext->RSSetState(gCullNoneState);

	// No need to set vertex/index buffer (see fullscreen quad vertex shader), just indicate that the quad will be created as a triangle strip
	gD3DContext->IASetInputLayout(NULL); // No vertex data
	gD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// Set 2D area for full-screen post-processing (coordinates in 0->1 range)
	gPostProcessingConstants.area2DTopLeft = { 0, 0 }; // Top-left of entire screen
	gPostProcessingConstants.area2DSize    = { 1, 1 }; // Full size of screen
	gPostProcessingConstants.area2DDepth   = 0;        // Depth buffer value for full screen is as close as possible

	// Shader settings
	gD3DContext->PSSetShader(postProcess, nullptr, 0u);

	if (gCurrentPostProcess == PostProcess::Tint)
	{
		gD3DContext->PSSetShader(gTintPostProcess, nullptr, 0);
		gPostProcessingConstants.tintColour = { 1, 0, 0 };
	}
	else if (gCurrentPostProcess == PostProcess::GreyNoise)
	{
		gD3DContext->PSSetShader(gGreyNoisePostProcess, nullptr, 0);

		// Noise scaling adjusts how fine the noise is.
		const float grainSize = 50; // Fineness of the noise grain
		gPostProcessingConstants.noiseScale = { gViewportWidth / grainSize, gViewportHeight / grainSize };

		// The noise offset is randomised to give a constantly changing noise effect (like tv static)
		gPostProcessingConstants.noiseOffset = { Random(0.0f, 1.0f), Random(0.0f, 1.0f) };

		// Give pixel shader access to the noise texture
		gD3DContext->PSSetShaderResources(1, 1, &gNoiseMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
	}
	else if (gCurrentPostProcess == PostProcess::Burn)
	{
		gD3DContext->PSSetShader(gBurnPostProcess, nullptr, 0);

		// Set and increase the burn level (cycling back to 0 when it reaches 1.0f)
		const float burnSpeed = 0.2f;
		gPostProcessingConstants.burnHeight = fmod(gPostProcessingConstants.burnHeight + burnSpeed * frameTime, 1.0f);

		// Give pixel shader access to the burn texture (basically a height map that the burn level ascends)
		gD3DContext->PSSetShaderResources(1, 1, &gBurnMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
	}
	else if (gCurrentPostProcess == PostProcess::Distort)
	{
		gD3DContext->PSSetShader(gDistortPostProcess, nullptr, 0);

		// Set the level of distortion
		gPostProcessingConstants.distortLevel = 0.03f;

		// Give pixel shader access to the distortion texture (containts 2D vectors (in R & G) to shift the texture UVs to give a cut-glass impression)
		gD3DContext->PSSetShaderResources(1, 1, &gDistortMapSRV);
		gD3DContext->PSSetSamplers(1, 1, &gTrilinearSampler);
	}
	else if (gCurrentPostProcess == PostProcess::Spiral)
	{
		gD3DContext->PSSetShader(gSpiralPostProcess, nullptr, 0);

		static float wiggle = 0.0f;
		const float wiggleSpeed = 1.0f;

		// Set and increase the amount of spiral - use a tweaked cos wave to animate
		gPostProcessingConstants.spiralLevel = ((1.0f - cos(wiggle)) * 4.0f);
		wiggle += wiggleSpeed * frameTime;
	}
	else if (gCurrentPostProcess == PostProcess::VColourGradient)
	{
		gD3DContext->PSSetShader(gVColourGradientPostProcess, nullptr, 0);
	}
	else if (gCurrentPostProcess == PostProcess::HLSGradient)
	{
		gD3DContext->PSSetShader(gHLSGradientPostProcess, nullptr, 0);

		static float wiggle = 0.0f;
		const float wiggleSpeed = 0.5f;
		gPostProcessingConstants.hueShift = wiggle;
		wiggle += wiggleSpeed * frameTime;
	}
	else if (gCurrentPostProcess == PostProcess::FullScreenBlur)
	{
		gD3DContext->PSSetShader(gFullScreenBlurPostProcess, nullptr, 0);
	}
	else if (gCurrentPostProcess == PostProcess::UnderWater)
	{
		gD3DContext->PSSetShader(gUnderWaterPostProcess, nullptr, 0);

		static float waterWiggle = 0.0f;
		const float waterWiggleSpeed = 0.5f;

		gPostProcessingConstants.underWaterLevel = waterWiggle;
		waterWiggle = waterWiggle + waterWiggleSpeed * frameTime;

	}
	else if (gCurrentPostProcess == PostProcess::Retro)
	{
		gD3DContext->PSSetShader(gRetroPostProcess, nullptr, 0);

	}
	else if (gCurrentPostProcess == PostProcess::GaussianBlur)
	{
		gD3DContext->PSSetShader(gGaussianBlurPostProcess, nullptr, 0);
	}
	else if (gCurrentPostProcess == PostProcess::Bloom)
	{
		gD3DContext->PSSetShader(gBloomPostProcess, nullptr, 0);
	}

	

	// Loop through the given points, transform each to 2D (this is what the vertex shader normally does in most labs)
	for (unsigned int i = 0; i < points.size(); ++i)
	{
		CVector4 modelPosition = CVector4(points[i], 1.0f);
		CVector4 worldPosition = TransformVector4(modelPosition, worldMatrix);
		CVector4 viewportPosition = TransformVector4(worldPosition, gCamera->ViewProjectionMatrix());

		gPostProcessingConstants.polygon2DPoints[i] = viewportPosition;
	}

	// Pass over the polygon points to the shaders (also sends the per-process settings prepared in UpdateScene function below)
	UpdateConstantBuffer(gPostProcessingConstantBuffer, gPostProcessingConstants);
	gD3DContext->VSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);
	gD3DContext->PSSetConstantBuffers(1, 1, &gPostProcessingConstantBuffer);

	// Select the special 2D polygon post-processing vertex shader and draw the polygon
	gD3DContext->VSSetShader(g2DPolygonVertexShader, nullptr, 0);

	gD3DContext->Draw(4, 0);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	gD3DContext->PSSetShaderResources(0, 1, &nullSRV);
}


// Rendering the scene
void RenderScene(float frameTime)
{
	//// Common settings ////

	// Set up the light information in the constant buffer
	// Don't send to the GPU yet, the function RenderSceneFromCamera will do that
	gPerFrameConstants.light1Colour   = gLights[0].colour * gLights[0].strength;
	gPerFrameConstants.light1Position = gLights[0].model->Position();
	gPerFrameConstants.light2Colour   = gLights[1].colour * gLights[1].strength;
	gPerFrameConstants.light2Position = gLights[1].model->Position();

	gPerFrameConstants.ambientColour  = gAmbientColour;
	gPerFrameConstants.specularPower  = gSpecularPower;
	gPerFrameConstants.cameraPosition = gCamera->Position();

	gPerFrameConstants.viewportWidth  = static_cast<float>(gViewportWidth);
	gPerFrameConstants.viewportHeight = static_cast<float>(gViewportHeight);

	gPerFrameConstants.frameTime = frameTime;

	////--------------- Main scene rendering ---------------////

	// Set the target for rendering and select the main depth buffer.
	// If using post-processing then render to the scene texture, otherwise to the usual back buffer
	// Also clear the render target to a fixed colour and the depth buffer to the far distance

	gD3DContext->OMSetRenderTargets(1, &gSceneRenderTarget1, gDepthStencil);
	gD3DContext->ClearRenderTargetView(gSceneRenderTarget1, &gBackgroundColor.r);
	gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Setup the viewport to the size of the main window
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<FLOAT>(gViewportWidth);
	vp.Height = static_cast<FLOAT>(gViewportHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	gD3DContext->RSSetViewports(1, &vp);

	// Render the scene from the main camera
	RenderSceneFromCamera(gCamera);


	////--------------- Scene completion ---------------////


	gCurrentTarget = gSceneRenderTarget2;
	gCurrentSRV = gSceneTextureSRV1;

	gCurrentPostProcess = PostProcess::None;
	PostProcessing(gCurrentTarget, gCurrentSRV, frameTime);
	SwapPostProcessEffect();

	// This is where you can change which process goes in the each window
	PolygonPostProcess(gHLSGradientPostProcess, PolygonPoints, SquareWindowPolygonMatrix,  frameTime);
	PolygonPostProcess(gUnderWaterPostProcess,  PolygonPoints, SpadeWindowPolygonMatrix,   frameTime);
	PolygonPostProcess(gRetroPostProcess,       PolygonPoints, DiamondWindowPolygonMatrix, frameTime);

	gCurrentPostProcess = PostProcess::Spiral;
	PolygonPostProcess(gSpiralPostProcess,      PolygonPoints, ClubWindowPolygonMatrix,    frameTime);

	gCurrentPostProcess = PostProcess::Distort;
	PolygonPostProcess(gDistortPostProcess,     PolygonPoints, HeartWindowPolygonMatrix,   frameTime);


	SwapPostProcessEffect();

	if (!postProcessEffectList.empty())
	{
		for (auto effect : postProcessEffectList)
		{
			gCurrentPostProcess = effect;
			PostProcessing(gCurrentTarget, gCurrentSRV, frameTime);
			SwapPostProcessEffect();
		}
	}

	gCurrentPostProcess = PostProcess::None;
	PostProcessing(gBackBufferRenderTarget, gCurrentSRV, frameTime);

	// When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
	// Set first parameter to 1 to lock to vsync
	gSwapChain->Present(lockFPS ? 1 : 0, 0);
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------

// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
	// Select post process on keys
	if (KeyHit(Key_1)) UpdatePostProcessEffectsList(PostProcess::HLSGradient);
	if (KeyHit(Key_2)) UpdatePostProcessEffectsList(PostProcess::GaussianBlur);
	if (KeyHit(Key_3)) UpdatePostProcessEffectsList(PostProcess::UnderWater);
	if (KeyHit(Key_4)) UpdatePostProcessEffectsList(PostProcess::Retro);
	if (KeyHit(Key_5)) UpdatePostProcessEffectsList(PostProcess::Bloom);

	if (KeyHit(Key_0)) ResetPostProcessEffectsList ();

	// Orbit one light - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
	static float lightRotate = 0.0f;
	static bool go = true;
	gLights[0].model->SetPosition({ 20 + cos(lightRotate) * gLightOrbitRadius, 10, 20 + sin(lightRotate) * gLightOrbitRadius });
	if (go)  lightRotate -= gLightOrbitSpeed * frameTime;
	if (KeyHit(Key_L))  go = !go;

	// Control of camera
	gCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D);

	// Toggle FPS limiting
	if (KeyHit(Key_P))  lockFPS = !lockFPS;

	// Show frame time / FPS in the window title //
	const float fpsUpdateTime = 0.5f; // How long between updates (in seconds)
	static float totalFrameTime = 0;
	static int frameCount = 0;
	totalFrameTime += frameTime;
	++frameCount;
	if (totalFrameTime > fpsUpdateTime)
	{
		// Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
		float avgFrameTime = totalFrameTime / frameCount;
		std::ostringstream frameTimeMs;
		frameTimeMs.precision(2);
		frameTimeMs << std::fixed << avgFrameTime * 1000;
		std::string windowTitle = "Post Processing Assignment - Frame Time: " + frameTimeMs.str() +
			"ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
		SetWindowTextA(gHWnd, windowTitle.c_str());
		totalFrameTime = 0;
		frameCount = 0;
	}
}

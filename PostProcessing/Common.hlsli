//--------------------------------------------------------------------------------------
// Common include file for all shaders
//--------------------------------------------------------------------------------------
// Using include files to define the type of data passed between the shaders


//--------------------------------------------------------------------------------------
// Shader input / output
//--------------------------------------------------------------------------------------

// The structure below describes the vertex data to be sent into the vertex shader for ordinary non-skinned models
struct BasicVertex
{
    float3 position : position;
    float3 normal : normal;
    float2 uv : uv;
};



// This structure describes what data the lighting pixel shader receives from the vertex shader.
// The projected position is a required output from all vertex shaders - where the vertex is on the screen
// The world position and normal at the vertex are sent to the pixel shader for the lighting equations.
// The texture coordinates (uv) are passed from vertex shader to pixel shader unchanged to allow textures to be sampled
struct LightingPixelShaderInput
{
    float4 projectedPosition : SV_Position; // This is the position of the pixel to render, this is a required input
                                            // to the pixel shader and so it uses the special semantic "SV_Position"
                                            // because the shader needs to identify this important information
    
    float3 worldPosition : worldPosition; // The world position and normal of each vertex is passed to the pixel...
    float3 worldNormal : worldNormal; //...shader to calculate per-pixel lighting. These will be interpolated
                                            // automatically by the GPU (rasterizer stage) so each pixel will know
                                            // its position and normal in the world - required for lighting equations
    
    float2 uv : uv; // UVs are texture coordinates. The artist specifies for every vertex which point on the texture is "pinned" to that vertex.
};


// This structure is similar to the one above but for the light models, which aren't themselves lit
struct SimplePixelShaderInput
{
    float4 projectedPosition : SV_Position;
    float2 uv : uv;
};



//**************************

// The vertex data received by each post-process shader. Just the 2d projected position (pixel coordinate on screen), 
// and UV for accessing the texture showing the scene
struct PostProcessingInput
{
    float4               projectedPosition : SV_Position;
    noperspective float2 sceneUV           : sceneUV; // "noperspective" is needed for polygon processing or the sampling of the scene texture doesn't work correctly (ask tutor if you are interested)
    float2               areaUV            : areaUV;
};

//**************************



//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------

// These structures are "constant buffers" - a way of passing variables over from C++ to the GPU
// They are called constants but that only means they are constant for the duration of a single GPU draw call.
// These "constants" correspond to variables in C++ that we will change per-model, or per-frame etc.

// In this exercise the matrices used to position the camera are updated from C++ to GPU every frame along with lighting information
// These variables must match exactly the gPerFrameConstants structure in Scene.cpp
cbuffer PerFrameConstants : register(b0) // The b0 gives this constant buffer the number 0 - used in the C++ code
{
    float4x4 gCameraMatrix; // World matrix for the camera (inverse of the ViewMatrix below)
    float4x4 gViewMatrix;
    float4x4 gProjectionMatrix;
    float4x4 gViewProjectionMatrix; // The above two matrices multiplied together to combine their effects

    float3 gLight1Position; // 3 floats: x, y z
    float  gViewportWidth; // Using viewport width and height as padding - see this structure in earlier labs to read about padding here
    float3 gLight1Colour;
    float  gViewportHeight;

    float3 gLight2Position;
    float  padding1;
    float3 gLight2Colour;
    float  padding2;

    float3 gAmbientColour;
    float  gSpecularPower;

    float3 gCameraPosition;
    float  gFrameTime; // This app does updates on the GPU so we pass over the frame update time
}
// Note constant buffers are not structs: we don't use the name of the constant buffer, these are really just a collection of global variables (hence the 'g')



static const int MAX_BONES = 64;

// If we have multiple models then we need to update the world matrix from C++ to GPU multiple times per frame because we
// only have one world matrix here. Because this data is updated more frequently it is kept in a different buffer for better performance.
// We also keep other data that changes per-model here
// These variables must match exactly the gPerModelConstants structure in Scene.cpp
cbuffer PerModelConstants : register(b1) // The b1 gives this constant buffer the number 1 - used in the C++ code
{
    float4x4 gWorldMatrix;

    float3   gObjectColour; // Useed for tinting light models
    float    gExplodeAmount; // Used in the geometry shader to control how much the polygons are exploded outwards

    float4x4 gBoneMatrices[MAX_BONES];
}


//**************************

// This is where we receive post-processing settings from the C++ side
// These variables must match exactly the gPostProcessingConstants structure in Scene.cpp
// Note that this buffer reuses the same index (register) as the per-model buffer above since they won't be used together
cbuffer PostProcessingConstants : register(b1) 
{
    float2 gArea2DTopLeft; // Top-left of post-process area on screen, provided as coordinate from 0.0->1.0 not as a pixel coordinate
    float2 gArea2DSize; // Size of post-process area on screen, provided as sizes from 0.0->1.0 (1 = full screen) not as a size in pixels
    float  gArea2DDepth; // Depth buffer value for area (0.0 nearest to 1.0 furthest). Full screen post-processing uses 0.0f
    float2 paddingA; // Pad things to collections of 4 floats (see notes in earlier labs to read about padding)

    float4 gPolygon2DPoints[4]; // Four points of a polygon in 2D viewport space for polygon post-processing. Matrix transformations already done on C++ side

    
	// Tint post-process settings
	float3 gTintColour;
	float  paddingB;  // Pad things to collections of 4 floats (see notes in earlier labs to read about padding)

	// Grey noise post-process settings
    float2 gNoiseScale;
	float2 gNoiseOffset;

	// Burn post-process settings
	float  gBurnHeight;
	float3 paddingC;

	// Distort post-process settings
	float  gDistortLevel;
	float3 paddingD;

	// Spiral post-process settings
	float  gSpiralLevel;
	float3 paddingE;
    
    // UnderWater post-process settings
    float gUnderWaterLevel;
    float3 paddingF;
    
    // HSl hue post-process setting
    float gHueShift;
    float3 paddingG;

    // Gaussian Blur  direction
    float2 gBlurDirection;
    float2 paddingH;
    
    bool gFeedbackBlur;

}

//**************************


//--------------------------------------------------------------------------------------
// Full Screen Post-Processing Vertex Shader
//--------------------------------------------------------------------------------------
// Vertex shader generates a full screen quad so the pixel shader can copy/process the scene texture to it

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// This rather unusual vertex shader generates its own vertices rather than reading them from a buffer.
// The only input data is a special value, the "vertex ID". This is an automatically generated increasing index starting at 0.
// It uses this index to create 4 points of a full screen quad (coordinates -1 to 1), it also generates texture
// coordinates because the post-processing will sample a texture containing a picture of the scene.
// No vertex or index buffer is required, which makes the C++ side simpler
PostProcessingInput main(uint vertexId : SV_VertexID)
{
	PostProcessingInput output; // Defined in Common.hlsi

	// Four corners of the screen in "projection space". X and Y coordinates are -1 to 1. (Z and W are for the depth buffer, not relevant here)
	const float4 QuadPositions[4] = { float4(-1.0, 1.0, 0.0, 1.0),
						              float4(-1.0,-1.0, 0.0, 1.0),
						              float4( 1.0, 1.0, 0.0, 1.0),
						              float4( 1.0,-1.0, 0.0, 1.0) };
	// Four corners of the screen as texture coordinates - so we can access the texture showing the scene in the pixel shader that follows
	const float2 QuadUVs[4] = { float2(0.0, 0.0),
						        float2(0.0, 1.0),
						        float2(1.0, 0.0),
						        float2(1.0, 1.0) };

	// Just look up the above arrays with the vertex ID
	output.projectedPosition = QuadPositions[vertexId];
	output.sceneUV = QuadUVs[vertexId];

	return output;
}

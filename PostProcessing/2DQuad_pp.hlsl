//--------------------------------------------------------------------------------------
// 2D Quad Post-Processing Vertex Shader
//--------------------------------------------------------------------------------------
// Vertex shader generates a 2D quad on the screen so the pixel shader can copy/process the scene texture to it
// Can be used for full-screen or area effects. For area effects the post process pixel shader should soften the 
// edges so the hard boundary of the quad is not visible (see heat haze shader for example)

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

	// Four corners of a quad. These are used together with the quad size values passed over in the constant buffer.
	// We calculate the screen coordinates of the screen quad being written to and the UV coordinates of the scene texture to use as a source
	const float2 Quad[4] = { float2(0.0, 0.0),   // Top-left
					         float2(0.0, 1.0),   // Bottom-left
						     float2(1.0, 0.0),   // Top-right
						     float2(1.0, 1.0) }; // Bottom-left

	// Look up the above array with the vertex ID (see comment on function)
	float2 quadCoord = Quad[vertexId];

	// Use values passed over from C++ in constant buffer (common.hlsli) to convert to coordinates of area to affect
	float2 areaCoord = gArea2DTopLeft + quadCoord * gArea2DSize;

	// Convert 0->1 UV coordinates to 2D viewport coordinates (-1 -> 1) and flip Y axis
	float2 screenCoord = areaCoord * 2 - 1;
	screenCoord.y = -screenCoord.y;

	// Pass values on to post-processing pixel shader, also set depth value for this post-process (set from C++)
	output.areaUV  = quadCoord;  // These UVs refer to the area being post-processed (see ascii diagram below)
	output.sceneUV = areaCoord;  // These UVs refer to the scene texture (see diagram below)
	output.projectedPosition = float4( screenCoord, gArea2DDepth, 1 );


	// We send two sets of UV coordinates to the post-processing shaders
	// - The uvScene coordinates indicate which pixels of the scene texture to sample
	//     - They are used to sample the pixels in the scene texture
	// - The uvArea coordinates indicate which part of the area itself we are in
	//     - Used so effects know where the edge of the affected area are for fading out or for sampling a secondary texture,
	//       such as the distortion or burn texture
	// - For full screen effects, these two coordinates are the same which is why they weren't present in the full-screen lab
	//
	// uvScene = (0,0) here
	//    +-------------------------------+
	//    |                               |
	//    |                               |
	//    |  uvArea = (0,0)               |
	//    |       +--------------+        |
	//    |       |              |        |
	//    |       |              |        |
	//    |       +--------------+        |
	//    |               uvArea = (1,1)  |
	//    |                               |
	//    |                               |
	//    +-------------------------------+
	//                               uvScene = (1,1) here
	return output;
}

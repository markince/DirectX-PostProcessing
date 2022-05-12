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

// This post-processing vertex shader expects that the C++ side will have already done all the matrix transformations for a four
// point polygon and passed the resultant four points via a constant buffer (rather than via the usual vertex buffer).
// A more flexible polygon post-procssing system would have vertex and index buffers etc. However for one-off effects (e.g. a frosted window), this is sufficient.
PostProcessingInput main(uint vertexId : SV_VertexID)
{
	PostProcessingInput output; // Defined in Common.hlsi

	// The four points of the polygon come from the C++ side. The shader gives the polygon fixed UVs
	const float2 polygonUVs[4] = { float2(0.0, 0.0),   // Top-left
					               float2(0.0, 1.0),   // Bottom-left
						           float2(1.0, 0.0),   // Top-right
						           float2(1.0, 1.0) }; // Bottom-left

	// The post-processing shaders expect the points of the polygon (came from C++), the UVs for the area to affect (in the array above)...
	// ... and the UVs of which part of the scene texture is getting affected. We don't have that yet but it can be caclulated from the...
	// ... x and y coordinates of the polygon points
	output.projectedPosition = gPolygon2DPoints[vertexId];
	output.areaUV = polygonUVs[vertexId];
	output.sceneUV = (output.projectedPosition.xy / output.projectedPosition.w + 1.0f) * 0.5f;
	output.sceneUV.y = 1.0f - output.sceneUV.y;

	return output;
}

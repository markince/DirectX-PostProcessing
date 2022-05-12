//--------------------------------------------------------------------------------------
// Class encapsulating a mesh
//--------------------------------------------------------------------------------------
// The mesh class splits the mesh into sub-meshes that only use one texture each.
// The class also doesn't load textures, filters or shaders as the outer code is
// expected to select these things. A later lab will introduce a more robust loader.

#include "Mesh.h"
#include "Shader.h" // Needed for helper function CreateSignatureForVertexLayout
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here
#include "CVector2.h" 
#include "CVector3.h" 

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>

#include <memory>


// Pass the name of the mesh file to load. Uses assimp (http://www.assimp.org/) to support many file types
// Optionally request tangents to be calculated (for normal and parallax mapping - see later lab)
// Will throw a std::runtime_error exception on failure (since constructors can't return errors).
Mesh::Mesh(const std::string& fileName, bool requireTangents /*= false*/)
{
	Assimp::Importer importer;

	// Flags for processing the mesh. Assimp provides a huge amount of control - right click any of these
	// and "Peek Definition" to see documention above each constant
	unsigned int assimpFlags = aiProcess_MakeLeftHanded |
		aiProcess_GenSmoothNormals |
		aiProcess_FixInfacingNormals |
		aiProcess_GenUVCoords |
		aiProcess_TransformUVCoords |
		aiProcess_FlipUVs |
		aiProcess_FlipWindingOrder |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_SortByPType |
		aiProcess_FindInvalidData |
		aiProcess_OptimizeMeshes |
		aiProcess_FindInstances |
		aiProcess_FindDegenerates |
		aiProcess_RemoveRedundantMaterials |
		aiProcess_Debone |
		aiProcess_SplitByBoneCount |
		aiProcess_LimitBoneWeights |
		aiProcess_RemoveComponent;

	// Flags to specify what mesh data to ignore
	int removeComponents = aiComponent_LIGHTS | aiComponent_CAMERAS | aiComponent_TEXTURES | aiComponent_COLORS |
		aiComponent_ANIMATIONS | aiComponent_MATERIALS;

	// Add / remove tangents as required by user
	if (requireTangents)
	{
		assimpFlags |= aiProcess_CalcTangentSpace;
	}
	else
	{
		removeComponents |= aiComponent_TANGENTS_AND_BITANGENTS;
	}

	// Other miscellaneous settings
	importer.SetPropertyFloat(AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, 80.0f); // Smoothing angle for normals
	importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);  // Remove points and lines (keep triangles only)
	importer.SetPropertyBool(AI_CONFIG_PP_FD_REMOVE, true);                 // Remove degenerate triangles
	importer.SetPropertyBool(AI_CONFIG_PP_DB_ALL_OR_NONE, true);            // Default to removing bones/weights from meshes that don't need skinning

	// Set maximum bones that can affect one vertex, and also maximum bones affecting a single mesh
	unsigned int maxBonesPerVertex = 4; // The shaders support 4 bones per verted (null bones are added if necessary)
	unsigned int maxBonesPerMesh = 256; // Bone indexes are stored in a byte, so no more than 256 
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, maxBonesPerVertex);
	importer.SetPropertyInteger(AI_CONFIG_PP_SBBC_MAX_BONES, maxBonesPerMesh);

	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, removeComponents);

	// Import mesh with assimp given above requirements - log output
	Assimp::DefaultLogger::create("", Assimp::DefaultLogger::VERBOSE);
	const aiScene* scene = importer.ReadFile(fileName, assimpFlags);
	Assimp::DefaultLogger::kill();
	if (scene == nullptr)  throw std::runtime_error("Error loading mesh (" + fileName + "). " + importer.GetErrorString());
	if (scene->mNumMeshes == 0)  throw std::runtime_error("No usable geometry in mesh: " + fileName);


	//-----------------------------------

	//*********************************************************************//
	// Read node hierachy - each node has a matrix and contains sub-meshes //

	// Uses recursive helper functions to build node hierarchy    
	mNodes.resize(CountNodes(scene->mRootNode));
	ReadNodes(scene->mRootNode, 0, 0);



	//******************************************//
	// Read geometry - multiple parts supported //

	mHasBones = false;
	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
		if (scene->mMeshes[m]->HasBones())  mHasBones = true;


	// A mesh is made of sub-meshes, each one can have a different material (texture)
	// Import each sub-mesh in the file to seperate index / vertex buffer (could share buffers between sub-meshes but that would make things more complex)
	mSubMeshes.resize(scene->mNumMeshes);
	for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
	{
		aiMesh* assimpMesh = scene->mMeshes[m];
		std::string subMeshName = assimpMesh->mName.C_Str();
		auto& subMesh = mSubMeshes[m]; // Short name for the submesh we're currently preparing - makes code below more readable


		//-----------------------------------

		// Check for presence of position and normal data. Tangents and UVs are optional.
		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexElements;
		unsigned int offset = 0;

		if (!assimpMesh->HasPositions())  throw std::runtime_error("No position data for sub-mesh " + subMeshName + " in " + fileName);
		unsigned int positionOffset = offset;
		vertexElements.push_back({ "position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, positionOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		offset += 12;

		if (!assimpMesh->HasNormals())  throw std::runtime_error("No normal data for sub-mesh " + subMeshName + " in " + fileName);
		unsigned int normalOffset = offset;
		vertexElements.push_back({ "normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, normalOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
		offset += 12;

		unsigned int tangentOffset = offset;
		if (requireTangents)
		{
			if (!assimpMesh->HasTangentsAndBitangents())  throw std::runtime_error("No tangent data for sub-mesh " + subMeshName + " in " + fileName);
			vertexElements.push_back({ "tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, tangentOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			offset += 12;
		}

		unsigned int uvOffset = offset;
		if (assimpMesh->GetNumUVChannels() > 0 && assimpMesh->HasTextureCoords(0))
		{
			if (assimpMesh->mNumUVComponents[0] != 2)  throw std::runtime_error("Unsupported texture coordinates in " + subMeshName + " in " + fileName);
			vertexElements.push_back({ "uv", 0, DXGI_FORMAT_R32G32_FLOAT, 0, uvOffset, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			offset += 8;
		}

		unsigned int bonesOffset = offset;
		if (mHasBones)
		{
			vertexElements.push_back({ "bones"  , 0, DXGI_FORMAT_R8G8B8A8_UINT,      0, bonesOffset,     D3D11_INPUT_PER_VERTEX_DATA, 0 });
			offset += 4;
			vertexElements.push_back({ "weights", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, bonesOffset + 4, D3D11_INPUT_PER_VERTEX_DATA, 0 });
			offset += 16;
		}

		subMesh.vertexSize = offset;


		// Create a "vertex layout" to describe to DirectX what is data in each vertex of this mesh
		auto shaderSignature = CreateSignatureForVertexLayout(vertexElements.data(), static_cast<int>(vertexElements.size()));
		HRESULT hr = gD3DDevice->CreateInputLayout(vertexElements.data(), static_cast<UINT>(vertexElements.size()),
			shaderSignature->GetBufferPointer(), shaderSignature->GetBufferSize(),
			&subMesh.vertexLayout);
		if (shaderSignature)  shaderSignature->Release();
		if (FAILED(hr))  throw std::runtime_error("Failure creating input layout for " + fileName);



		//-----------------------------------

		// Create CPU-side buffers to hold current mesh data - exact content is flexible so can't use a structure for a vertex - so just a block of bytes
		// Note: for large arrays a unique_ptr is better than a vector because vectors default-initialise all the values which is a waste of time.
		subMesh.numVertices = assimpMesh->mNumVertices;
		subMesh.numIndices = assimpMesh->mNumFaces * 3;
		auto vertices = std::make_unique<unsigned char[]>(subMesh.numVertices * subMesh.vertexSize);
		auto indices  = std::make_unique<unsigned char[]>(subMesh.numIndices * 4); // Using 32 bit indexes (4 bytes) for each indeex


		//-----------------------------------

		// Copy mesh data from assimp to our CPU-side vertex buffer

		CVector3* assimpPosition = reinterpret_cast<CVector3*>(assimpMesh->mVertices);
		unsigned char* position = vertices.get() + positionOffset;
		unsigned char* positionEnd = position + subMesh.numVertices * subMesh.vertexSize;
		while (position != positionEnd)
		{
			*(CVector3*)position = *assimpPosition;
			position += subMesh.vertexSize;
			++assimpPosition;
		}

		CVector3* assimpNormal = reinterpret_cast<CVector3*>(assimpMesh->mNormals);
		unsigned char* normal = vertices.get() + normalOffset;
		unsigned char* normalEnd = normal + subMesh.numVertices * subMesh.vertexSize;
		while (normal != normalEnd)
		{
			*(CVector3*)normal = *assimpNormal;
			normal += subMesh.vertexSize;
			++assimpNormal;
		}

		if (requireTangents)
		{
			CVector3* assimpTangent = reinterpret_cast<CVector3*>(assimpMesh->mTangents);
			unsigned char* tangent = vertices.get() + tangentOffset;
			unsigned char* tangentEnd = tangent + subMesh.numVertices * subMesh.vertexSize;
			while (tangent != tangentEnd)
			{
				*(CVector3*)tangent = *assimpTangent;
				tangent += subMesh.vertexSize;
				++assimpTangent;
			}
		}

		if (assimpMesh->GetNumUVChannels() > 0 && assimpMesh->HasTextureCoords(0))
		{
			aiVector3D* assimpUV = assimpMesh->mTextureCoords[0];
			unsigned char* uv = vertices.get() + uvOffset;
			unsigned char* uvEnd = uv + subMesh.numVertices * subMesh.vertexSize;
			while (uv != uvEnd)
			{
				*(CVector2*)uv = CVector2(assimpUV->x, assimpUV->y);
				uv += subMesh.vertexSize;
				++assimpUV;
			}
		}


		if (mHasBones)
		{
			if (assimpMesh->HasBones())
			{
				// Set all bones and weights to 0 to start with
				unsigned char* bones = vertices.get() + bonesOffset;
				unsigned char* bonesEnd = bones + subMesh.numVertices * subMesh.vertexSize;
				while (bones != bonesEnd)
				{
					memset(bones, 0, 20);
					bones += subMesh.vertexSize;
				}

				for (auto& node : mNodes)
				{
					node.offsetMatrix = MatrixIdentity();
				}

				// Go through each assimp bone
				bones = vertices.get() + bonesOffset;
				for (unsigned int i = 0; i < assimpMesh->mNumBones; ++i)
				{
					// Get offset matrix for the bone (transform from skinned mesh root to bone root
					aiBone* assimpBone = assimpMesh->mBones[i];
					std::string boneName = assimpBone->mName.C_Str();
					unsigned int nodeIndex;
					for (nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
					{
						if (mNodes[nodeIndex].name == boneName)
						{
							mNodes[nodeIndex].offsetMatrix.SetValues(&assimpBone->mOffsetMatrix.a1);
							mNodes[nodeIndex].offsetMatrix.Transpose(); // Assimp stores matrices differently to this app
							break;
						}
					}
					if (nodeIndex == mNodes.size())  throw std::runtime_error("Bone with no matching node in " + fileName);

					// Go through each weight of the bone and update the vertex it influences
					// Find the first 0 weight on that vertex and put the new influence / weight there.
					// A vertex can only have up to 4 influences
					for (unsigned int j = 0; j < assimpBone->mNumWeights; ++j)
					{
						unsigned int vertexIndex = assimpBone->mWeights[j].mVertexId;
						unsigned char* bone = bones + vertexIndex * subMesh.vertexSize;
						float* weight = (float*)(bone + 4);
						float* lastWeight = weight + 3;
						while (*weight != 0.0f && weight != lastWeight)
						{
							bone++; weight++;
						}
						if (*weight == 0.0f)
						{
							*bone = nodeIndex;
							*weight = assimpBone->mWeights[j].mWeight;
						}
					}
				}
			}
			else
			{
				// In a mesh that uses skinning any sub-meshes that don't contain bones are given bones so the whole mesh can use one shader
				unsigned int subMeshNode = 0;
				for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
				{
					for (auto& subMeshIndex : mNodes[nodeIndex].subMeshes)
					{
						if (subMeshIndex == m)
							subMeshNode = nodeIndex;
					}
				}

				unsigned char* bones = vertices.get() + bonesOffset;
				unsigned char* bonesEnd = bones + subMesh.numVertices * subMesh.vertexSize;
				while (bones != bonesEnd)
				{
					memset(bones, 0, 20);
					bones[0] = subMeshNode;
					*(float*)(bones + 4) = 1.0f;
					bones += subMesh.vertexSize;
				}

			}
		}



		//-----------------------------------

		// Copy face data from assimp to our CPU-side index buffer
		if (!assimpMesh->HasFaces())  throw std::runtime_error("No face data in " + subMeshName + " in " + fileName);

		DWORD* index = reinterpret_cast<DWORD*>(indices.get());
		for (unsigned int face = 0; face < assimpMesh->mNumFaces; ++face)
		{
			*index++ = assimpMesh->mFaces[face].mIndices[0];
			*index++ = assimpMesh->mFaces[face].mIndices[1];
			*index++ = assimpMesh->mFaces[face].mIndices[2];
		}


		//-----------------------------------

		D3D11_BUFFER_DESC bufferDesc;
		D3D11_SUBRESOURCE_DATA initData;

		// Create GPU-side vertex buffer and copy the vertices imported by assimp into it
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Indicate it is a vertex buffer
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;          // Default usage for this buffer - we'll see other usages later
		bufferDesc.ByteWidth = subMesh.numVertices * subMesh.vertexSize; // Size of the buffer in bytes
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		initData.pSysMem = vertices.get(); // Fill the new vertex buffer with data loaded by assimp

		hr = gD3DDevice->CreateBuffer(&bufferDesc, &initData, &subMesh.vertexBuffer);
		if (FAILED(hr))  throw std::runtime_error("Failure creating vertex buffer for " + fileName);


		// Create GPU-side index buffer and copy the vertices imported by assimp into it
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // Indicate it is an index buffer
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;         // Default usage for this buffer - we'll see other usages later
		bufferDesc.ByteWidth = subMesh.numIndices * sizeof(DWORD); // Size of the buffer in bytes
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		initData.pSysMem = indices.get(); // Fill the new index buffer with data loaded by assimp

		hr = gD3DDevice->CreateBuffer(&bufferDesc, &initData, &subMesh.indexBuffer);
		if (FAILED(hr))  throw std::runtime_error("Failure creating index buffer for " + fileName);
	}
}


Mesh::~Mesh()
{
	for (auto& subMesh : mSubMeshes)
	{
		if (subMesh.indexBuffer)   subMesh.indexBuffer ->Release();
		if (subMesh.vertexBuffer)  subMesh.vertexBuffer->Release();
		if (subMesh.vertexLayout)  subMesh.vertexLayout->Release();
	}
}


//--------------------------------------------------------------------------------------

// Helper function for Render function - renders a given sub-mesh. World matrices / textures / states etc. must already be set
void Mesh::RenderSubMesh(const SubMesh& subMesh)
{
	// Set vertex buffer as next data source for GPU
	UINT stride = subMesh.vertexSize;
	UINT offset = 0;
	gD3DContext->IASetVertexBuffers(0, 1, &subMesh.vertexBuffer, &stride, &offset);

	// Indicate the layout of vertex buffer
	gD3DContext->IASetInputLayout(subMesh.vertexLayout);

	// Set index buffer as next data source for GPU, indicate it uses 32-bit integers
	gD3DContext->IASetIndexBuffer(subMesh.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Using triangle lists only in this class
	gD3DContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Render mesh
	gD3DContext->DrawIndexed(subMesh.numIndices, 0, 0);
}



// Render the mesh with the given matrices
// Handles rigid body meshes (including single part meshes) as well as skinned meshes
// LIMITATION: The mesh must use a single texture throughout
void Mesh::Render(std::vector<CMatrix4x4>& modelMatrices)
{
	// Skinning needs all matrices available in the shader at the same time, so first calculate all the absolute
	// matrices before rendering anything
	std::vector<CMatrix4x4> absoluteMatrices(modelMatrices.size());
	absoluteMatrices[0] = modelMatrices[0]; // First matrix for a model is the root matrix, already in world space
	for (unsigned int nodeIndex = 1; nodeIndex < mNodes.size(); ++nodeIndex)
	{
		// Multiply each model matrix by its parent's absolute world matrix (already calculated earlier in this loop)
		// Same process as for rigid bodies, simply done prior to rendering now
		absoluteMatrices[nodeIndex] = modelMatrices[nodeIndex] * absoluteMatrices[mNodes[nodeIndex].parentIndex];
	}

	if (mHasBones) // Render a mesh that uses skinning
	{
		// Advanced point: the above loop will get the absolute world matrices **of the bones**. However, they are
		// not actually rendered, they merely influence the skinned mesh, which has its origin at a particular node.
		// So for each bone there is a fixed offset (transform) between where that bone is and where the root of the
		// skinned mesh is. We need to apply that offset to each of the bone matrices calculated in the last loop to make
		// the bone influences work on the skinned mesh.
		// These offset matrices are fixed for the model and have been calculated when the mesh was imported
		for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
		{
			absoluteMatrices[nodeIndex] = mNodes[nodeIndex].offsetMatrix * absoluteMatrices[nodeIndex];
		}

		// Send all matrices over to the GPU for skinning via a constant buffer - each matrix can represent a bone which influences nearby vertices
		// MISSING - code to fill the gPerModelConstants.boneMatrices array with the contents of the absoluteMatrices vector
		for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
		{
			gPerModelConstants.boneMatrices[nodeIndex] = absoluteMatrices[nodeIndex];
		}
		UpdateConstantBuffer(gPerModelConstantBuffer, gPerModelConstants); // Send to GPU

		// Indicate that the constant buffer we just updated is for use in the vertex shader (VS), geometry shader (GS) and pixel shader (PS)
		gD3DContext->VSSetConstantBuffers(1, 1, &gPerModelConstantBuffer); // First parameter must match constant buffer number in the shader
		gD3DContext->GSSetConstantBuffers(1, 1, &gPerModelConstantBuffer); // First parameter must match constant buffer number in the shader
		gD3DContext->PSSetConstantBuffers(1, 1, &gPerModelConstantBuffer);

		// Already sent over all the absolute matrices for the entire mesh so we can render sub-meshes directly
		// rather than iterating through the nodes. 
		for (auto& subMesh : mSubMeshes)
		{
			RenderSubMesh(subMesh);
		}
	}
	else
	{
		// Render a mesh without skinning. Although slightly reorganised to use the matrices calculated
		// above, this is basically the same code as the rigid body animation lab
		// Iterate through each node
		for (unsigned int nodeIndex = 0; nodeIndex < mNodes.size(); ++nodeIndex)
		{
			// Send this node's matrix to the GPU via a constant buffer
			gPerModelConstants.worldMatrix = absoluteMatrices[nodeIndex];
			UpdateConstantBuffer(gPerModelConstantBuffer, gPerModelConstants); // Send to GPU

			// Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
			gD3DContext->VSSetConstantBuffers(1, 1, &gPerModelConstantBuffer); // First parameter must match constant buffer number in the shader
			gD3DContext->GSSetConstantBuffers(1, 1, &gPerModelConstantBuffer);
			gD3DContext->PSSetConstantBuffers(1, 1, &gPerModelConstantBuffer);

			// Render the sub-meshes attached to this node (no bones - rigid movement)
			for (auto& subMeshIndex : mNodes[nodeIndex].subMeshes)
			{
				RenderSubMesh(mSubMeshes[subMeshIndex]);
			}
		}
	}
}


//--------------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------------

// Count the number of nodes with given assimp node as root - recursive
unsigned int Mesh::CountNodes(aiNode* assimpNode)
{
	unsigned int count = 1;
	for (unsigned int child = 0; child < assimpNode->mNumChildren; ++child)
		count += CountNodes(assimpNode->mChildren[child]);
	return count;
}


// Help build the arrays of submeshes and nodes from the assimp data - recursive
unsigned int Mesh::ReadNodes(aiNode* assimpNode, unsigned int nodeIndex, unsigned int parentIndex)
{
	auto& node = mNodes[nodeIndex];
	node.parentIndex = parentIndex;
	unsigned int thisIndex = nodeIndex;
	++nodeIndex;

	node.name = assimpNode->mName.C_Str();

	node.defaultMatrix.SetValues(&assimpNode->mTransformation.a1);
	node.defaultMatrix.Transpose(); // Assimp stores matrices differently to this app

	node.subMeshes.resize(assimpNode->mNumMeshes);
	for (unsigned int i = 0; i < assimpNode->mNumMeshes; ++i)
	{
		node.subMeshes[i] = assimpNode->mMeshes[i];
	}

	node.childNodes.resize(assimpNode->mNumChildren);
	for (unsigned int i = 0; i < assimpNode->mNumChildren; ++i)
	{
		node.childNodes[i] = nodeIndex;
		nodeIndex = ReadNodes(assimpNode->mChildren[i], nodeIndex, thisIndex);
	}

	return nodeIndex;
}

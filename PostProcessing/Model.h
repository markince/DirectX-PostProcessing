//--------------------------------------------------------------------------------------
// Class encapsulating a model
//--------------------------------------------------------------------------------------
// Holds a pointer to a mesh as well as position, rotation and scaling, which are converted to a world matrix when required
// This is more of a convenience class, the Mesh class does most of the difficult work.

#include "CVector3.h"
#include "CMatrix4x4.h"
#include "Input.h"

#include <vector>

#ifndef _MODEL_H_INCLUDED_
#define _MODEL_H_INCLUDED_

class Mesh;

class Model
{
public:
	//-------------------------------------
	// Construction / Usage
	//-------------------------------------

    Model(Mesh* mesh, CVector3 position = { 0,0,0 }, CVector3 rotation = { 0,0,0 }, float scale = 1);


    // The render function simply passes this model's matrices over to Mesh:Render.
    // All other per-frame constants must have been set already along with shaders, textures, samplers, states etc.
    void Render();


	// Control a given node in the model using keys provided. Amount of motion performed depends on frame time
	void Control(int node, float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,  
				                            KeyCode turnCW, KeyCode turnCCW, KeyCode moveForward, KeyCode moveBackward );


	//-------------------------------------
	// Data access
	//-------------------------------------

    //********************************
    // All functions now accept a "node" parameter which specifies which node in the hierarchy to use. Defaults to 0, the root.
    // The hierarchy is stored in depth-first order

	// Getters - model only stores matrices. Position, rotation and scale are extracted if requested.
	CVector3 Position(int node = 0)  { return mWorldMatrices[node].GetRow(3); }         // Position is on bottom row of matrix
	CVector3 Rotation(int node = 0)  { return mWorldMatrices[node].GetEulerAngles(); }  // Getting angles from a matrix is complex - see .cpp file
	CVector3 Scale(int node = 0)     { return { Length(mWorldMatrices[node].GetRow(0)),
                                                Length(mWorldMatrices[node].GetRow(1)), 
                                                Length(mWorldMatrices[node].GetRow(2)) }; } // Scale is length of rows 0-2 in matrix
	CMatrix4x4 WorldMatrix(int node = 0)  { return mWorldMatrices[node]; }

    // Setters - model only stores matricies , so if user sets position, rotation or scale, just update those aspects of the matrix
	void SetPosition(CVector3 position, int node = 0)  { mWorldMatrices[node].SetRow(3, position); }

	void SetRotation(CVector3 rotation, int node = 0)
    {
        // To put rotation angles into a matrix we need to build the matrix from scratch to make sure we retain existing scaling and position
        mWorldMatrices[node] = MatrixScaling(Scale(node)) *
                               MatrixRotationZ(rotation.z) * MatrixRotationX(rotation.x) * MatrixRotationY(rotation.y) *
                               MatrixTranslation(Position(node));
    }

	// Two ways to set scale: x,y,z separately, or all to the same value
    // To set scale without affecting rotation, normalise each row, then multiply it by the scale value.
	void SetScale(CVector3 scale, int node = 0)
    {
        mWorldMatrices[node].SetRow(0, Normalise(mWorldMatrices[node].GetRow(0)) * scale.x); 
        mWorldMatrices[node].SetRow(1, Normalise(mWorldMatrices[node].GetRow(1)) * scale.y); 
        mWorldMatrices[node].SetRow(2, Normalise(mWorldMatrices[node].GetRow(2)) * scale.z); 
    }
	void SetScale(float scale)  { SetScale({ scale, scale, scale });}

    void SetWorldMatrix(CMatrix4x4 matrix, int node = 0)  { mWorldMatrices[node] = matrix; }


	//-------------------------------------
	// Private data / members
	//-------------------------------------
private:
    Mesh* mMesh;

	// World matrices for the model
    // Now that meshes have multiple parts, we need multiple matrices. The root matrix (the first one) is the world matrix
    // for the entire model. The remaining matrices are relative to their parent part. The hierarchy is defined in the mesh (nodes)
	std::vector<CMatrix4x4> mWorldMatrices;
};


#endif //_MODEL_H_INCLUDED_

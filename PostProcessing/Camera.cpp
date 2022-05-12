//--------------------------------------------------------------------------------------
// Class encapsulating a camera
//--------------------------------------------------------------------------------------
// Holds position, rotation, near/far clip and field of view. These to a view and projection matrices as required

#include "Camera.h"
#include "Common.h"

// Control the camera's position and rotation using keys provided
void Camera::Control(float frameTime, KeyCode turnUp, KeyCode turnDown, KeyCode turnLeft, KeyCode turnRight,
                                      KeyCode moveForward, KeyCode moveBackward, KeyCode moveLeft, KeyCode moveRight)
{
	//**** ROTATION ****
	if (KeyHeld(Key_Down))
	{
		mRotation.x += ROTATION_SPEED * frameTime; // Use of frameTime to ensure same speed on different machines
	}
	if (KeyHeld(Key_Up))
	{
		mRotation.x -= ROTATION_SPEED * frameTime;
	}
	if (KeyHeld(Key_Right))
	{
		mRotation.y += ROTATION_SPEED * frameTime;
	}
	if (KeyHeld(Key_Left))
	{
		mRotation.y -= ROTATION_SPEED * frameTime;
	}

	//**** LOCAL MOVEMENT ****
	if (KeyHeld(Key_D))
	{
		mPosition.x += MOVEMENT_SPEED * frameTime * mWorldMatrix.e00; // See comments on local movement in UpdateCube code above
		mPosition.y += MOVEMENT_SPEED * frameTime * mWorldMatrix.e01; 
		mPosition.z += MOVEMENT_SPEED * frameTime * mWorldMatrix.e02; 
	}
	if (KeyHeld(Key_A))
	{
		mPosition.x -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e00;
		mPosition.y -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e01;
		mPosition.z -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e02;
	}
	if (KeyHeld(Key_W))
	{
		mPosition.x += MOVEMENT_SPEED * frameTime * mWorldMatrix.e20;
		mPosition.y += MOVEMENT_SPEED * frameTime * mWorldMatrix.e21;
		mPosition.z += MOVEMENT_SPEED * frameTime * mWorldMatrix.e22;
	}
	if (KeyHeld(Key_S))
	{
		mPosition.x -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e20;
		mPosition.y -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e21;
		mPosition.z -= MOVEMENT_SPEED * frameTime * mWorldMatrix.e22;
	}
}


// Update the matrices used for the camera in the rendering pipeline
void Camera::UpdateMatrices()
{
    // "World" matrix for the camera - treat it like a model at first
    mWorldMatrix = MatrixRotationZ(mRotation.z) * MatrixRotationX(mRotation.x) * MatrixRotationY(mRotation.y) * MatrixTranslation(mPosition);

    // View matrix is the usual matrix used for the camera in shaders, it is the inverse of the world matrix (see lectures)
    mViewMatrix = InverseAffine(mWorldMatrix);

    // Projection matrix, how to flatten the 3D world onto the screen (needs field of view, near and far clip, aspect ratio)
    float tanFOVx = std::tan(mFOVx * 0.5f);
    float scaleX = 1.0f / tanFOVx;
    float scaleY = mAspectRatio / tanFOVx;
    float scaleZa = mFarClip / (mFarClip - mNearClip);
    float scaleZb = -mNearClip * scaleZa;

    mProjectionMatrix = { scaleX,   0.0f,    0.0f,   0.0f,
                            0.0f, scaleY,    0.0f,   0.0f,
                            0.0f,   0.0f, scaleZa,   1.0f,
                            0.0f,   0.0f, scaleZb,   0.0f };

    // The view-projection matrix combines the two matrices usually used for the camera into one, which can save a multiply in the shaders (optional)
    mViewProjectionMatrix = mViewMatrix * mProjectionMatrix;
}


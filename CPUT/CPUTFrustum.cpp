//--------------------------------------------------------------------------------------
// Copyright 2012 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
//--------------------------------------------------------------------------------------
#include "CPUTFrustum.h"
#include "CPUTCamera.h"

CPUTFrustum::CPUTFrustum()
{
	mPlanes = (float *) _aligned_malloc(4 * 2 * sizeof(__m128), sizeof(__m128));
}

//-----------------------------------------------
CPUTFrustum::~CPUTFrustum()
{
	_aligned_free(mPlanes);
}

//-----------------------------------------------
void CPUTFrustum::InitializeFrustum( CPUTCamera *pCamera )
{
    InitializeFrustum(
        pCamera->GetNearPlaneDistance(),
        pCamera->GetFarPlaneDistance(),
        pCamera->GetAspectRatio(),
        pCamera->GetFov(),
        pCamera->GetPosition(),
        pCamera->GetLook(),
        pCamera->GetUp()
    );

}

//-----------------------------------------------
void CPUTFrustum::InitializeFrustum
(
    float nearClipDistance,
    float farClipDistance,
    float aspectRatio,
    float fov,
    const float3 &position,
    const float3 &look,
    const float3 &up
)
{
    // ******************************
    // This function computes the position of each of the frustum's eight points.
    // It also computes the normal of each of the frustum's six planes.
    // ******************************

    mNumFrustumVisibleModels = 0;
    mNumFrustumCulledModels  = 0;

    // We have the camera's up and look, but we also need right.
    float3 right = cross3( up, look );

    // Compute the position of the center of the near and far clip planes.
    float3 nearCenter = position + look * nearClipDistance;
    float3 farCenter  = position + look * farClipDistance;

    // Compute the width and height of the near and far clip planes
    float tanHalfFov = tanf(0.5f*fov);
    float halfNearWidth  = nearClipDistance * tanHalfFov;
    float halfNearHeight = halfNearWidth / aspectRatio;
    
    float halfFarWidth   = farClipDistance * tanHalfFov;
    float halfFarHeight  = halfFarWidth / aspectRatio;
    
    // Create two vectors each for the near and far clip planes.
    // These are the scaled up and right vectors.
    float3 upNear      = up    * halfNearHeight;
    float3 rightNear   = right * halfNearWidth;
    float3 upFar       = up    * halfFarHeight;
    float3 rightFar    = right * halfFarWidth;

    // Use the center positions and the up and right vectors
    // to compute the positions for the near and far clip plane vertices (four each)
    mpPosition[0] = nearCenter + upNear - rightNear; // near top left
    mpPosition[1] = nearCenter + upNear + rightNear; // near top right
    mpPosition[2] = nearCenter - upNear + rightNear; // near bottom right
    mpPosition[3] = nearCenter - upNear - rightNear; // near bottom left
    mpPosition[4] = farCenter  + upFar  - rightFar;  // far top left
    mpPosition[5] = farCenter  + upFar  + rightFar;  // far top right
    mpPosition[6] = farCenter  - upFar  + rightFar;  // far bottom right
    mpPosition[7] = farCenter  - upFar  - rightFar;  // far bottom left

    // Compute some of the frustum's edge vectors.  We will cross these
    // to get the normals for each of the six planes.
    float3 nearTop     = mpPosition[1] - mpPosition[0];
    float3 nearLeft    = mpPosition[3] - mpPosition[0];
    float3 topLeft     = mpPosition[4] - mpPosition[0];
    float3 bottomRight = mpPosition[2] - mpPosition[6];
    float3 farRight    = mpPosition[5] - mpPosition[6];
    float3 farBottom   = mpPosition[7] - mpPosition[6];

    mpNormal[0] = cross3(nearTop,     nearLeft).normalize();    // near clip plane
    mpNormal[1] = cross3(nearLeft,    topLeft).normalize();     // left
    mpNormal[2] = cross3(topLeft,     nearTop).normalize();     // top
    mpNormal[3] = cross3(farBottom,   bottomRight).normalize(); // bottom
    mpNormal[4] = cross3(bottomRight, farRight).normalize();    // right
    mpNormal[5] = cross3(farRight,    farBottom).normalize();   // far clip plane

	for (int i=0; i < 6; i++)
	{
		mPlanes[0*8 + i] = mpNormal[i].x;
		mPlanes[1*8 + i] = mpNormal[i].y;
		mPlanes[2*8 + i] = mpNormal[i].z;
		mPlanes[3*8 + i] = -dot3(mpNormal[i], mpPosition[(i < 3) ? 0 : 6]);
	}

	for (int i=6; i < 8; i++)
	{
		mPlanes[0*8 + i] = 0;
		mPlanes[1*8 + i] = 0;
		mPlanes[2*8 + i] = 0;
		mPlanes[3*8 + i] = -1.0f;
	}
}

//-----------------------------------------------
bool CPUTFrustum::IsVisible(
    const float3 &center,
    const float3 &half
){
    UINT ii;
    float3 absHalf = abs3(half);

    float3 planeToPoint = center - mpPosition[0]; // Use near-clip-top-left point for point on first three planes
    for( ii=0; ii<3; ii++ )
    {
        float3 normal      = mpNormal[ii];
        float3 absNormal   = abs3(normal);
        float  nDotC       = dot3( normal, planeToPoint );
        if( nDotC > dot3( abs3(normal), absHalf ) )
        {
            return false;
        }
    }

    planeToPoint = center - mpPosition[6]; // Use near-clip-top-left point for point on first three planes
    for( ii=3; ii<6; ii++ )
    {
        float3 normal      = mpNormal[ii];
        float3 absNormal   = abs3(normal);
        float  nDotC       = dot3( normal, planeToPoint );
        if( nDotC > dot3( abs3(normal), absHalf ) )
        {
            return false;
        }
    }

    // Tested all eight points against all six planes and none of the planes
    // had all eight points outside.
    return true;
}


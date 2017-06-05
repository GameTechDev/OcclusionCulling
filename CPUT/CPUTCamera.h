////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License.  You may obtain a copy
// of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
////////////////////////////////////////////////////////////////////////////////
#ifndef __CPUTCamera_H__
#define __CPUTCamera_H__

#include <memory.h>
#include <Windows.h>
#include "CPUT.h"
#include "CPUTRenderNode.h"
#include "CPUTConfigBlock.h"
#include "CPUTFrustum.h"

//-----------------------------------------------------------------------------
class CPUTCamera:public CPUTRenderNode
{
protected:
    float mFov;                // the field of view in degrees
    float mNearPlaneDistance;  // near plane distance
    float mFarPlaneDistance;   // far plane distance
    float mAspectRatio;        // width/height.  TODO: Support separate pixel and viewport aspect ratios

    float4x4 mView;
    float4x4 mProjection;

public:
    CPUTFrustum mFrustum;

    CPUTCamera();
    ~CPUTCamera() {}

    void Update( float deltaSeconds=0.0f ) {
        // TODO: Do only if required (i.e. if dirty)
        mProjection = float4x4PerspectiveFovLH( mFov, mAspectRatio, mFarPlaneDistance, mNearPlaneDistance );
        mView = inverse(*GetWorldMatrix());
        mFrustum.InitializeFrustum(this);
    };

    CPUTResult LoadCamera(CPUTConfigBlock *pBlock, int *pParentID);

    float4x4 *GetViewMatrix(void)
    {
        // Update();  We can't afford to do this every time we're asked for the view matrix.  Caller needs to make sure camera is updated before entering render loop.
        return &mView;
    }

    const float4x4* GetProjectionMatrix(void) const { return &mProjection; }
    void            SetProjectionMatrix(const float4x4 &projection) { mProjection = projection; }
    float           GetAspectRatio() { return mAspectRatio; }
    float           GetFov() { return mFov; }
    void            SetAspectRatio(const float aspectRatio);
    void            SetFov( const float fov );
    float           GetNearPlaneDistance() { return mNearPlaneDistance; }
    float           GetFarPlaneDistance() {  return mFarPlaneDistance; }
    void            SetNearPlaneDistance( const float nearPlaneDistance ) { mNearPlaneDistance = nearPlaneDistance; }
    void            SetFarPlaneDistance(  const float farPlaneDistance ) { mFarPlaneDistance = farPlaneDistance; }
    void            LookAt( float xx, float yy, float zz );
	CPUTCamera &operator=(const CPUTCamera& camera);
};

//-----------------------------------------------------------------------------
class CPUTCameraController : public CPUTEventHandler
{
protected:
    CPUTRenderNode *mpCamera;
    float           mfMoveSpeed;
    float           mfLookSpeed;
    int             mnPrevFrameX;
    int             mnPrevFrameY;
    CPUTMouseState  mPrevFrameState;

public:
    CPUTCameraController()
        : mpCamera(NULL)
        , mnPrevFrameX(0)
        , mnPrevFrameY(0)
        , mfMoveSpeed(1.0f)
        , mfLookSpeed(1.0f)
    {
    }
    ~CPUTCameraController(){ SAFE_RELEASE(mpCamera);}
    void            SetCamera(CPUTRenderNode *pCamera)  { SAFE_RELEASE(mpCamera); mpCamera = pCamera; pCamera->AddRef(); }
    CPUTRenderNode *GetCamera(void) const               { return mpCamera; }
    void            SetMoveSpeed(float speed)           { mfMoveSpeed = speed; }
    void            SetLookSpeed(float speed)           { mfLookSpeed = speed; }
    virtual void    Update(float deltaSeconds=0.0f) = 0;
};

// TODO: Move these implementations to the .cpp file.
//-----------------------------------------------------------------------------
class CPUTCameraControllerFPS : public CPUTCameraController
{
public:
    void Update( float deltaSeconds=0.0f);
    // TODO: Change to Update(deltaSeconds) and IsKeyDown()
    CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key) { return CPUT_EVENT_UNHANDLED; }
    CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state);
};

//-----------------------------------------------------------------------------
class CPUTCameraControllerArcBall : public CPUTCameraController
{
public:
    void Update( float deltaSeconds=0.0f ) {}
    CPUTEventHandledCode HandleKeyboardEvent(CPUTKey key) { return CPUT_EVENT_UNHANDLED; }
    CPUTEventHandledCode HandleMouseEvent(int x, int y, int wheel, CPUTMouseState state);
};

#endif //#ifndef __CPUTCamera_H__

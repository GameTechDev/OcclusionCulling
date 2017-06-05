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
#ifndef __CPUTMODEL_H__
#define __CPUTMODEL_H__

// Define the following to support drawing bounding boxes.
// Note that you also need bounding-box materials and shaders.  TODO: Include them in the exe.
#define SUPPORT_DRAWING_BOUNDING_BOXES 1

#include "CPUTRenderNode.h"
#include "CPUTMath.h"
#include "CPUTConfigBlock.h"
#include "CPUTMesh.h"

class CPUTMaterial;
class CPUTMesh;

//-----------------------------------------------------------------------------
class CPUTModel : public CPUTRenderNode
{
protected:  
    static CPUTMaterial  *mpShadowCastMaterialMaster;
    static CPUTMaterial  *mpBoundingBoxMaterialMaster;
    static CPUTMesh      *mpBoundingBoxMesh;

    CPUTMesh     **mpMesh;
    CPUTMaterial **mpMaterial;
    CPUTMaterial  *mpShadowCastMaterial;

    UINT           mMeshCount;
    bool           mIsRenderable;
    float3         mBoundingBoxCenterObjectSpace;
    float3         mBoundingBoxHalfObjectSpace;
    float3         mBoundingBoxCenterWorldSpace;
    float3         mBoundingBoxHalfWorldSpace;
    CPUTMaterial  *mpBoundingBoxMaterial;

public:
    CPUTModel():
        mMeshCount(0),
        mpMesh(NULL),
        mIsRenderable(true),
        mBoundingBoxCenterObjectSpace(0.0f),
        mBoundingBoxHalfObjectSpace(0.0f),
        mBoundingBoxCenterWorldSpace(0.0f),
        mBoundingBoxHalfWorldSpace(0.0f),
        mpBoundingBoxMaterial(NULL),
        mpShadowCastMaterial(NULL)
    {}
    virtual ~CPUTModel();
    static void ReleaseStaticResources();

    bool               IsRenderable() { return mIsRenderable; }
    void               SetRenderable(bool isRenderable) { mIsRenderable = isRenderable; }
    virtual bool       IsModel() { return true; }
    void               GetBoundsObjectSpace(float3 *pCenter, float3 *pHalf);
    void               GetBoundsWorldSpace(float3 *pCenter, float3 *pHalf);
    void               UpdateBoundsWorldSpace();
    int                GetMeshCount() const { return mMeshCount; }
    CPUTMesh          *GetMesh( UINT ii ) { return mpMesh[ii]; }
    virtual CPUTResult LoadModel(CPUTConfigBlock *pBlock, int *pParentID, CPUTModel *pMasterModel=NULL) = 0;
    CPUTResult         LoadModelPayload(const cString &File);
    virtual void       SetMaterial(UINT ii, CPUTMaterial *pMaterial);
#ifdef SUPPORT_DRAWING_BOUNDING_BOXES
    virtual void       DrawBoundingBox(CPUTRenderParameters &renderParams) = 0;
    virtual void       CreateBoundingBoxMesh() = 0;
#endif

    void GetBoundingBoxRecursive( float3 *pCenter, float3 *pHalf)
    {
        if( *pHalf == float3(0.0f) )
        {
            *pCenter = mBoundingBoxCenterWorldSpace;
            *pHalf   = mBoundingBoxHalfWorldSpace;
        }
        else
        {
            float3 minExtent = *pCenter - *pHalf;
            float3 maxExtent = *pCenter + *pHalf;
            minExtent = Min( (mBoundingBoxCenterWorldSpace - mBoundingBoxHalfWorldSpace), minExtent );
            maxExtent = Max( (mBoundingBoxCenterWorldSpace + mBoundingBoxHalfWorldSpace), maxExtent );
            *pCenter = (minExtent + maxExtent) * 0.5f;
            *pHalf   = (maxExtent - minExtent) * 0.5f;
        }
        if(mpChild)   { mpChild->GetBoundingBoxRecursive(   pCenter, pHalf ); }
        if(mpSibling) { mpSibling->GetBoundingBoxRecursive( pCenter, pHalf ); }
    }
    CPUTMaterial *GetMaterial( const cString name, int meshIndex ) const;
    CPUTMaterial *GetMaterial( int meshIndex ) const { return mpMaterial[meshIndex]; }
    CPUTBuffer   *GetBuffer( const cString &name, int meshIndex ) const;
};
#endif // __CPUTMODEL_H__
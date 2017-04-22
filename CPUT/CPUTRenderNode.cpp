/////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2017 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");// you may not use this file except in compliance with the License.// You may obtain a copy of the License at//// http://www.apache.org/licenses/LICENSE-2.0//// Unless required by applicable law or agreed to in writing, software// distributed under the License is distributed on an "AS IS" BASIS,// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.// See the License for the specific language governing permissions and// limitations under the License.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "CPUTRenderNode.h"

#include "CPUTOSServicesWin.h" // for OutputDebugString();


// Constructor
//-----------------------------------------------------------------------------
CPUTRenderNode::CPUTRenderNode():
    mpParent(NULL),
    mpChild(NULL),
    mpSibling(NULL)
{
    // set transform to identity
    mWorldMatrix  = float4x4Identity();
    mParentMatrix = float4x4Identity();

    // always create with everything dirty
    mWorldMatrixDirty = true;
}

// Destructor
//-----------------------------------------------------------------------------
CPUTRenderNode::~CPUTRenderNode()
{
    SAFE_RELEASE(mpParent);
    SAFE_RELEASE(mpChild);
    SAFE_RELEASE(mpSibling);
}

//-----------------------------------------------------------------------------
int CPUTRenderNode::ReleaseRecursive()
{
// #define OUTPUT_DEBUG_INFO
#ifdef OUTPUT_DEBUG_INFO
    char pRefCountString[64];
    sprintf_s( pRefCountString, 64, "(%d):", GetRefCount()-1 );
    OutputDebugStringA( pRefCountString );
    cString msg = GetName() + _L("\n");
    OutputDebugString( msg.c_str() );

    if( mpParent )
    {
        OutputDebugString( _L("Parent: ") );
        sprintf_s( pRefCountString, 64, "(%d):", mpParent->GetRefCount()-1  );
        OutputDebugStringA( pRefCountString );
        cString msg =  (mpParent ? mpParent->GetName() : _L("NULL\n")) + _L("\n");
        OutputDebugString( msg.c_str() );
    }
#endif
    // Release the parent.  Note: we don't want to recursively release it, or it would release us = infinite loop.
    SAFE_RELEASE(mpParent);

    // Recursively release our children and siblings
    if( mpChild )
    {
#ifdef OUTPUT_DEBUG_INFO
        OutputDebugString( _L("Child") ); 
#endif
        if( !mpChild->ReleaseRecursive() )
        {
            mpChild = NULL;
        }
    }
    if( mpSibling )
    {
#ifdef OUTPUT_DEBUG_INFO
        OutputDebugString( _L("Sibling:") );
#endif
        int refCount = mpSibling->ReleaseRecursive();
        if( !refCount )
        {
            mpSibling = NULL;
        }
    }
    return CPUTRefCount::Release();
}

// parent/child/sibling
//-----------------------------------------------------------------------------
void CPUTRenderNode::SetParent(CPUTRenderNode *pParent)
{
    SAFE_RELEASE(mpParent);
    if(NULL!=pParent)
    {
        pParent->AddRef();
    }
    mpParent = pParent;
}

//-----------------------------------------------------------------------------
void CPUTRenderNode::AddChild(CPUTRenderNode *pNode )
{
    ASSERT( NULL != pNode, _L("Can't add NULL node.") );
    if( mpChild )
    {
        mpChild->AddSibling( pNode );
    }
    else
    {
        pNode->AddRef();
        mpChild = pNode;
    }
}

//-----------------------------------------------------------------------------
void CPUTRenderNode::AddSibling(CPUTRenderNode *pNode )
{
    ASSERT( NULL != pNode, _L("Can't add NULL node.") );

    if( mpSibling )
    {
        mpSibling->AddSibling( pNode );
    }
    else
    {
        mpSibling = pNode;
        pNode->AddRef();
    }
}

// Return the model's cumulative transform
//-----------------------------------------------------------------------------
float4x4* CPUTRenderNode::GetWorldMatrix()
{
    if(mWorldMatrixDirty)
    {
        if(NULL!=mpParent)
        {
            float4x4 *pParentWorldMatrix = mpParent->GetWorldMatrix();
            mWorldMatrix = mParentMatrix * *pParentWorldMatrix;
        }
        else
        {
            mWorldMatrix = mParentMatrix;
        }
       mWorldMatrixDirty = false;
    }

    // copy it
    return &mWorldMatrix;
}

// Recursively visit all sub-nodes in breadth-first mode and mark their
// cumulative transforms as dirty
//-----------------------------------------------------------------------------
void CPUTRenderNode::MarkDirty()
{
    mWorldMatrixDirty = true;

    if(mpSibling)
    {
        mpSibling->MarkDirty();
    }

    if(mpChild)
    {
        mpChild->MarkDirty();
    }
}

// Update - recursively visit all sub-nodes in breadth-first mode
// Likely used for animation with a frame# or timestamp passed in
// so that the update routine would calculate the new transforms
// and called before Render() function
//-----------------------------------------------------------------------------
void CPUTRenderNode::UpdateRecursive( float deltaSeconds )
{
    // TODO: Need to Update this node first.
    Update(deltaSeconds);

    if(mpSibling)
    {
        mpSibling->UpdateRecursive(deltaSeconds);
    }
    if(mpChild)
    {
        mpChild->UpdateRecursive(deltaSeconds);
    }
}

// RenderRecursive - recursively visit all sub-nodes in breadth-first mode
//-----------------------------------------------------------------------------
void CPUTRenderNode::RenderRecursive(CPUTRenderParameters &renderParams)
{
    Render(renderParams);

    if(mpChild)
    {
        mpChild->RenderRecursive(renderParams);
        CPUTRenderNode *pNode = mpChild->GetSibling();
        while(pNode)
        {
            pNode->RenderRecursive(renderParams);
            pNode = pNode->GetSibling();
        }
    }
}

// RenderRecursive - recursively visit all sub-nodes in breadth-first mode
//-----------------------------------------------------------------------------
void CPUTRenderNode::RenderShadowRecursive(CPUTRenderParameters &renderParams)
{
    RenderShadow(renderParams);

    if(mpChild)
    {
        mpChild->RenderShadowRecursive(renderParams);
        CPUTRenderNode *pNode = mpChild->GetSibling();
        while(pNode)
        {
            pNode->RenderShadowRecursive(renderParams);
            pNode = pNode->GetSibling();
        }
    }
}


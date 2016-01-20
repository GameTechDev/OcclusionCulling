//--------------------------------------------------------------------------------------
// Copyright 2013 Intel Corporation
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
#include "CPUTModel.h"
#include "CPUTMaterial.h"
#include "CPUTMesh.h"
#include "CPUTAssetLibrary.h"
#include "CPUTBuffer.h"

CPUTMaterial  *CPUTModel::mpShadowCastMaterialMaster = NULL;
CPUTMaterial  *CPUTModel::mpBoundingBoxMaterialMaster = NULL;
CPUTMesh      *CPUTModel::mpBoundingBoxMesh=NULL;

//-----------------------------------------------------------------------------
CPUTModel::~CPUTModel()
{
    // TODO: Do we need a shadowCastMaterial per mesh?  What happens if the meshes have different layouts?
    HEAPCHECK;
    SAFE_RELEASE(mpBoundingBoxMaterial);
    HEAPCHECK;
    SAFE_RELEASE(mpShadowCastMaterial);
    HEAPCHECK;
    for( UINT ii=0; ii<mMeshCount; ii++ )
    {
        SAFE_RELEASE(mpMaterial[ii]);
        HEAPCHECK;
        if( mpMesh[ii] )
        {
            mpMesh[ii]->DecrementInstanceCount();
        }
        SAFE_RELEASE(mpMesh[ii]);
        HEAPCHECK;
    }
    SAFE_DELETE_ARRAY(mpMaterial);
    SAFE_DELETE_ARRAY(mpMesh);
}

//-----------------------------------------------------------------------------
void CPUTModel::ReleaseStaticResources()
{
    SAFE_RELEASE( CPUTModel::mpShadowCastMaterialMaster );
    SAFE_RELEASE( CPUTModel::mpBoundingBoxMaterialMaster);
    SAFE_RELEASE( CPUTModel::mpBoundingBoxMesh );
}

//-----------------------------------------------------------------------------
void CPUTModel::GetBoundsObjectSpace(float3 *pCenter, float3 *pHalf)
{
    *pCenter = mBoundingBoxCenterObjectSpace;
    *pHalf   = mBoundingBoxHalfObjectSpace;
}

//-----------------------------------------------------------------------------
void CPUTModel::GetBoundsWorldSpace(float3 *pCenter, float3 *pHalf)
{
    *pCenter = mBoundingBoxCenterWorldSpace;
    *pHalf   = mBoundingBoxHalfWorldSpace;
}

//-----------------------------------------------------------------------------
void CPUTModel::UpdateBoundsWorldSpace()
{
    // If an object is rigid, then it's object-space bounding box doesn't change.
    // However, if it moves, then it's world-space bounding box does change.
    // Call this function when the model moves

    float4x4 *pWorld =  GetWorldMatrix();
    float4 center    =  float4(mBoundingBoxCenterObjectSpace, 1.0f); // W = 1 because we want the xlation (i.e., center is a position)
    float4 half      =  float4(mBoundingBoxHalfObjectSpace,   0.0f); // W = 0 because we don't want xlation (i.e., half is a direction)

    // TODO: optimize this
    float4 positions[8] = {
        center + float4( 1.0f, 1.0f, 1.0f, 0.0f ) * half,
        center + float4( 1.0f, 1.0f,-1.0f, 0.0f ) * half,
        center + float4( 1.0f,-1.0f, 1.0f, 0.0f ) * half,
        center + float4( 1.0f,-1.0f,-1.0f, 0.0f ) * half,
        center + float4(-1.0f, 1.0f, 1.0f, 0.0f ) * half,
        center + float4(-1.0f, 1.0f,-1.0f, 0.0f ) * half,
        center + float4(-1.0f,-1.0f, 1.0f, 0.0f ) * half,
        center + float4(-1.0f,-1.0f,-1.0f, 0.0f ) * half
    };

    float4 minPosition( FLT_MAX,  FLT_MAX,  FLT_MAX, 1.0f );
    float4 maxPosition(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f );
    for( UINT ii=0; ii<8; ii++ )
    {
        float4 position = positions[ii] * *pWorld;
        minPosition = Min( minPosition, position );
        maxPosition = Max( maxPosition, position );
    }
    mBoundingBoxCenterWorldSpace = (maxPosition + minPosition) * 0.5f;
    mBoundingBoxHalfWorldSpace   = (maxPosition - minPosition) * 0.5f;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTModel::LoadModelPayload(const cString &File)
{
    CPUTResult result = CPUT_SUCCESS;

    std::ifstream file(File.c_str(), std::ios::in | std::ios::binary);
    ASSERT( !file.fail(), _L("CPUTModelDX11::LoadModelPayload() - Could not find binary model file: ") + File );

    // set up for mesh creation loop
    UINT meshIndex = 0;
    while(file.good() && !file.eof())
    {
        // TODO: rearrange while() to avoid if(eof).  Should perform only one branch per loop iteration, not two
        CPUTRawMeshData vertexFormatDesc;
        vertexFormatDesc.Read(file);
        if(file.eof())
        {
            // TODO:  Wtf?  Why would we get here?  We check eof at the top of loop.  If it isn't eof there, why is it eof here?
            break;
        }
        ASSERT( meshIndex < mMeshCount, _L("Actual mesh count doesn't match stated mesh count"));

        // create the mesh.
        CPUTMesh *pMesh = mpMesh[meshIndex];

        // always a triangle list (at this point)
        pMesh->SetMeshTopology(CPUT_TOPOLOGY_INDEXED_TRIANGLE_LIST);

        // get number of data blocks in the vertex element (pos,norm,uv,etc)
        // YUCK! TODO: Use fixed-size array of elements
        CPUTBufferInfo *pVertexElementInfo = new CPUTBufferInfo[vertexFormatDesc.mFormatDescriptorCount];
        // pMesh->SetBounds(vertexFormatDesc.mBboxCenter, vertexFormatDesc.mBboxHalf);

        // running count of each type of  element
        int positionStreamCount=0;
        int normalStreamCount=0;
        int texCoordStreamCount=0;
        int tangentStreamCount=0;
        int binormalStreamCount=0;
        int colorStreamCount=0;

        int RunningOffset = 0;
        for(UINT ii=0; ii<vertexFormatDesc.mFormatDescriptorCount; ii++)
        {
            // lookup the CPUT data type equivalent
            pVertexElementInfo[ii].mElementType = CPUT_FILE_ELEMENT_TYPE_TO_CPUT_TYPE_CONVERT[vertexFormatDesc.mpElements[ii].mVertexElementType];
            ASSERT((pVertexElementInfo[ii].mElementType !=CPUT_UNKNOWN ) , _L(".MDL file load error.  This model file has an unknown data type in it's model data."));
            // calculate the number of elements in this stream block (i.e. F32F32F32 = 3xF32)
            pVertexElementInfo[ii].mElementComponentCount = vertexFormatDesc.mpElements[ii].mElementSizeInBytes/CPUT_DATA_FORMAT_SIZE[pVertexElementInfo[ii].mElementType];
            // store the size of each element type in bytes (i.e. 3xF32, each element = F32 = 4 bytes)
            pVertexElementInfo[ii].mElementSizeInBytes = vertexFormatDesc.mpElements[ii].mElementSizeInBytes;
            // store the number of elements (i.e. 3xF32, 3 elements)
            pVertexElementInfo[ii].mElementCount = vertexFormatDesc.mVertexCount;
            // calculate the offset from the first element of the stream - assumes all blocks appear in the vertex stream as the order that appears here
            pVertexElementInfo[ii].mOffset = RunningOffset;
            RunningOffset = RunningOffset + pVertexElementInfo[ii].mElementSizeInBytes;

            // extract the name of stream
            pVertexElementInfo[ii].mpSemanticName = CPUT_VERTEX_ELEMENT_SEMANTIC_AS_STRING[ii];

            switch(vertexFormatDesc.mpElements[ii].mVertexElementSemantic)
            {
            case CPUT_VERTEX_ELEMENT_POSITON:
                pVertexElementInfo[ii].mpSemanticName = "POSITION";
                pVertexElementInfo[ii].mSemanticIndex = positionStreamCount++;
                break;
            case CPUT_VERTEX_ELEMENT_NORMAL:
                pVertexElementInfo[ii].mpSemanticName = "NORMAL";
                pVertexElementInfo[ii].mSemanticIndex = normalStreamCount++;
                break;
            case CPUT_VERTEX_ELEMENT_TEXTURECOORD:
                pVertexElementInfo[ii].mpSemanticName = "TEXCOORD";
                pVertexElementInfo[ii].mSemanticIndex = texCoordStreamCount++;
                break;
            case CPUT_VERTEX_ELEMENT_TANGENT:
                pVertexElementInfo[ii].mpSemanticName = "TANGENT";
                pVertexElementInfo[ii].mSemanticIndex = tangentStreamCount++;
                break;
            case CPUT_VERTEX_ELEMENT_BINORMAL:
                pVertexElementInfo[ii].mpSemanticName = "BINORMAL";
                pVertexElementInfo[ii].mSemanticIndex = binormalStreamCount++;
                break;
            case CPUT_VERTEX_ELEMENT_VERTEXCOLOR:
                pVertexElementInfo[ii].mpSemanticName = "COLOR";
                pVertexElementInfo[ii].mSemanticIndex = colorStreamCount++;
                break;
            default:
                cString errorString = _L("Invalid vertex semantic in: '")+File+_L("'\n");
                TRACE(errorString.c_str());
                ASSERT(0, errorString);
            }
        }

        // Index buffer
        CPUTBufferInfo indexDataInfo;
        indexDataInfo.mElementType           = (vertexFormatDesc.mIndexType == tUINT32) ? CPUT_U32 : CPUT_U16;
        indexDataInfo.mElementComponentCount = 1;
        indexDataInfo.mElementSizeInBytes    = (vertexFormatDesc.mIndexType == tUINT32) ? sizeof(UINT32) : sizeof(UINT16);
        indexDataInfo.mElementCount          = vertexFormatDesc.mIndexCount;
        indexDataInfo.mOffset                = 0;
        indexDataInfo.mSemanticIndex         = 0;
        indexDataInfo.mpSemanticName         = NULL;

        if( pVertexElementInfo->mElementCount && indexDataInfo.mElementCount )
        {
            result = pMesh->CreateNativeResources(
                this,
                meshIndex,
                vertexFormatDesc.mFormatDescriptorCount,
                pVertexElementInfo,
                (void*)vertexFormatDesc.mpVertices,
                &indexDataInfo,
                &vertexFormatDesc.mpIndices[0]
            );
            if(CPUTFAILED(result))
            {
                return result;
            }

			// CC added
			result = pMesh->ExtractVerticesandIndices();
			if(CPUTFAILED(result))
			{
				return result;  
			}
			// CC added ends
        }
        delete [] pVertexElementInfo;
        pVertexElementInfo = NULL;
        ++meshIndex;
    }
    ASSERT( file.eof(), _L("") );

    // close file
    file.close();

    return result;
}

// Set the material associated with this mesh and create/re-use a
//-----------------------------------------------------------------------------
void CPUTModel::SetMaterial(UINT ii, CPUTMaterial *pMaterial)
{
    // TODO: ASSSERT that ii is in range

    // release old material pointer
    SAFE_RELEASE( mpMaterial[ii] );

    mpMaterial[ii] = pMaterial;
    if(mpMaterial[ii])
    {
        mpMaterial[ii]->AddRef();
    }
}

//--------------------------------------------------------------------------------------
CPUTBuffer *CPUTModel::GetBuffer( const cString &name, int meshIndex ) const
{
    if( meshIndex >= 0 )
    {
        CPUTMaterial *pMaterial = mpMaterial[meshIndex];
        if( pMaterial )
        {
            UINT bufferCount = pMaterial->GetBufferCount();
            for( UINT ii=0; ii<bufferCount; ii++ )
            {
                CPUTBuffer *pBuffer = pMaterial->GetBuffer(ii);
                if( 0 == _wcsicmp( pBuffer->GetName().data(), name.data() ) )
                {
                    pBuffer->AddRef();
                    return pBuffer;
                }
            }
        }
    }
    else
    {
        // No meshIndex specified, so check all materials
        for(UINT ii=0; ii<mMeshCount; ii++ )
        {
            CPUTBuffer *pBuffer = GetBuffer( name, ii );
            if( pBuffer )
            {
                return pBuffer;
            }
        }
    }
    return NULL;
}

//--------------------------------------------------------------------------------------
CPUTMaterial *CPUTModel::GetMaterial( const cString name, int meshIndex ) const
{
    if( meshIndex >= 0 )
    {
        return mpMaterial[meshIndex];
    }
    else if( meshIndex == -2 )
    {
        return mpShadowCastMaterial;
    }
    else if( meshIndex == -3 )
    {
        return mpBoundingBoxMaterial;
    }
    else
    {
        // Loop over all materials.  If material name matches name param, then return the material.
        for( UINT ii=0; ii<mMeshCount; ii++ )
        {
            if( mpMaterial[ii] && (0 == _wcsicmp( name.data(), mpMaterial[ii]->GetMaterialName()->data() )) )
            {
                return mpMaterial[ii];
            }
        }
    }
    return NULL;
}

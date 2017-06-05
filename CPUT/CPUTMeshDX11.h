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
#ifndef __CPUTMESHDX11_H__
#define __CPUTMESHDX11_H__

#pragma once

#include "CPUTMesh.h"
#include "CPUTRenderParamsDX.h"
#include "CPUTInputLayoutCacheDX11.h"
#include "CPUT.h"
#include "CPUTOSServicesWin.h"

class CPUTMaterial;
class CPUTMaterialDX11;
class CPUTBufferDX11;
class CPUTComputeShaderDX11;
class CPUTModelDX11;

struct Vertex
{
	float3 pos;
};

//-----------------------------------------------------------------------------
class CPUTMeshDX11 : public CPUTMesh
{protected:
    D3D_PRIMITIVE_TOPOLOGY    mD3DMeshTopology;
    D3D11_INPUT_ELEMENT_DESC *mpLayoutDescription;
    int                       mNumberOfInputLayoutElements;
    ID3D11InputLayout        *mpInputLayout;
    ID3D11InputLayout        *mpShadowInputLayout;
    UINT                      mVertexStride;

    D3D11_BUFFER_DESC         mVertexBufferDesc;
    UINT                      mVertexBufferOffset;
    UINT                      mVertexCount;
    ID3D11Buffer             *mpVertexBuffer;
    ID3D11Buffer             *mpStagingVertexBuffer;
    eCPUTMapType              mVertexBufferMappedType;
    ID3D11Buffer             *mpVertexBufferForSRVDX; // Need SRV, but _real_ DX won't allow for _real_ VB
    ID3D11ShaderResourceView *mpVertexView;
    CPUTBufferDX11           *mpVertexBufferForSRV;

	// CC added 
	void					 *mpVertexData;
	Vertex					 *mpRawVertices;
	unsigned int        mRawVertexCount;
	// CC added end

    UINT                      mIndexCount;
    DXGI_FORMAT               mIndexBufferFormat;
    ID3D11Buffer             *mpIndexBuffer;
    D3D11_BUFFER_DESC         mIndexBufferDesc;
    ID3D11Buffer             *mpStagingIndexBuffer;
    eCPUTMapType              mIndexBufferMappedType;

	// CC added
	unsigned int			  mIndexElementByteSize;
	void					 *mpIndexData;
	unsigned int			 *mpRawIndices;
	static UINT				  mDrawCallCount;
	// CC added end

public:
    CPUTMeshDX11();
    virtual ~CPUTMeshDX11();

    D3D11_INPUT_ELEMENT_DESC *GetLayoutDescription() { return mpLayoutDescription; }
    ID3D11Buffer             *GetIndexBuffer()  { return mpIndexBuffer; }
    ID3D11Buffer             *GetVertexBuffer() { return mpVertexBuffer; }
    void                      SetMeshTopology(const eCPUT_MESH_TOPOLOGY eDrawTopology);
    CPUTResult                CreateNativeResources( CPUTModel *pModel, UINT meshIdx, int vertexDataInfoArraySize, CPUTBufferInfo *pVertexInfo, void *pVertexData, CPUTBufferInfo *pIndexInfo, void *pIndex );
    void                      BindVertexShaderLayout(CPUTMaterial *pMaterial, CPUTMaterial *pShadowCastMaterial);
    void                      Draw(CPUTRenderParameters &renderParams, CPUTModel *pModel)       { Draw(renderParams, pModel, mpInputLayout);}
    void                      DrawShadow(CPUTRenderParameters &renderParams, CPUTModel *pModel) { Draw(renderParams, pModel, mpShadowInputLayout);}
    void                      Draw(CPUTRenderParameters &renderParams, CPUTModel *pModel, ID3D11InputLayout *pLayout);

    D3D11_MAPPED_SUBRESOURCE  MapVertices(   CPUTRenderParameters &params, eCPUTMapType type, bool wait=true );
    D3D11_MAPPED_SUBRESOURCE  MapIndices(    CPUTRenderParameters &params, eCPUTMapType type, bool wait=true );
    void                      UnmapVertices( CPUTRenderParameters &params );
    void                      UnmapIndices(  CPUTRenderParameters &params );
    UINT                      GetTriangleCount() { return mIndexCount/3; }
    UINT                      GetVertexCount() { return mVertexCount; }
    UINT                      GetIndexCount()  { return mIndexCount; }

	// CC added
	unsigned int			 *GetIndices() { return mpRawIndices; }
	Vertex					 *GetVertices() { return mpRawVertices; }
	unsigned int        GetDepthVertexCount() { return mRawVertexCount; }
	unsigned int       *GetDepthIndices() { return mpRawIndices; }
	Vertex           *GetDepthVertices() { return mpRawVertices; }
	CPUTResult ExtractVerticesandIndices();

	static void ResetDrawCallCount(){mDrawCallCount = 0;}
	static UINT GetDrawCallCount(){return mDrawCallCount;}
	// CC added ends 

protected:
    // Mapping vertex and index buffers is very similar.  This internal function does both
    D3D11_MAPPED_SUBRESOURCE Map(
        UINT                   count,
        ID3D11Buffer          *pBuffer,
        D3D11_BUFFER_DESC     &bufferDesc,
        ID3D11Buffer         **pStagingBuffer,
        eCPUTMapType          *pMappedType,
        CPUTRenderParameters  &params,
        eCPUTMapType           type,
        bool                   wait = true
    );
    void  Unmap(
        ID3D11Buffer         *pBuffer,
        ID3D11Buffer         *pStagingBuffer,
        eCPUTMapType         *pMappedType,
        CPUTRenderParameters &params
    );
    void ClearAllObjects(); // delete all allocations held by this object
    DXGI_FORMAT ConvertToDirectXFormat(CPUT_DATA_FORMAT_TYPE DataFormatElementType, int NumberDataFormatElements);
	// CC added 
	CPUTResult GetByteSizeFromFormat(const DXGI_FORMAT& dxgiFormat, int &sizeInBytes);
	// CC added end
};

#endif // __CPUTMESHDX11_H__
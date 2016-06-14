//--------------------------------------------------------------------------------------
// Copyright 2015 Intel Corporation
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
#ifndef DEPTHBUFFERMASKEDRASTERIZERAVXST_H
#define DEPTHBUFFERMASKEDRASTERIZERAVXST_H

#include "DepthBufferRasterizerAVX.h"

class MaskedOcclusionCulling;

class DepthBufferMaskedRasterizerAVXST : public DepthBufferRasterizerAVX
{
	public:
		DepthBufferMaskedRasterizerAVXST(MaskedOcclusionCulling *moc);
		~DepthBufferMaskedRasterizerAVXST();

		void TransformModelsAndRasterizeToDepthBuffer(CPUTCamera *pCamera, UINT idx);
		void ComputeR2DBTime(UINT idx);

	private:
		void ActiveModels(UINT idx);
		void TransformAndRasterizeMeshes(UINT idx);

		MaskedOcclusionCulling *mMaskedOcclusionCulling;
};

#endif  //DEPTHBUFFERMASKEDRASTERIZERAVXST_H
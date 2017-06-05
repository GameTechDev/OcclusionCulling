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

#ifndef AABBOXMASKEDRASTERIZERAVXST_H
#define AABBOXMASKEDRASTERIZERAVXST_H

#include "AABBoxRasterizerAVX.h"

class MaskedOcclusionCulling;

class AABBoxMaskedRasterizerAVXST : public AABBoxRasterizerAVX
{
	public:
		AABBoxMaskedRasterizerAVXST(MaskedOcclusionCulling *moc);
		~AABBoxMaskedRasterizerAVXST();

		void TransformAABBoxAndDepthTest(CPUTCamera *pCamera, UINT idx);
		void WaitForTaskToFinish(UINT idx);
		void ReleaseTaskHandles(UINT idx);
	private:
		MaskedOcclusionCulling *mMaskedOcclusionCulling;
};

#endif //AABBOXMASKEDRASTERIZERAVXST_H
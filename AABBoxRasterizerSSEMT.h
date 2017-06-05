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

#ifndef AABBOXRASTERIZERSSEMT_H
#define AABBOXRASTERIZERSSEMT_H

#include "AABBoxRasterizerSSE.h"

class AABBoxRasterizerSSEMT : public AABBoxRasterizerSSE
{
	public:
		AABBoxRasterizerSSEMT();
		~AABBoxRasterizerSSEMT();

		struct PerTaskData
		{
			UINT idx;
			AABBoxRasterizerSSEMT *pAABB; 
		};

		PerTaskData mTaskData[2];
		void TransformAABBoxAndDepthTest(CPUTCamera *pCamera, UINT idx);
		void WaitForTaskToFinish(UINT idx);
		void ReleaseTaskHandles(UINT idx);

	private:
		static void TransformAABBoxAndDepthTest(VOID* pTaskData, INT context, UINT taskId, UINT taskCount);
		void TransformAABBoxAndDepthTest(UINT taskId, UINT taskCount, UINT idx);
};

#endif //AABBOXRASTERIZERSSEMT_H
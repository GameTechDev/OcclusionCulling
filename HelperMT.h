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

#ifndef HELPERMT_H
#define HELPERMT_H

static inline void GetWorkExtent(unsigned int *pstart, unsigned int *pend, unsigned int taskId, unsigned int taskCount, unsigned int workItemCount)
{
	unsigned int numRemainingItems = workItemCount % taskCount;

	unsigned int numItemsPerTask1 = workItemCount / taskCount + 1;
	unsigned int numItemsPerTask2 = workItemCount / taskCount;

	unsigned int start, end;
	
	if(taskId < numRemainingItems)
	{
		start = taskId * numItemsPerTask1;
		end   = start +  numItemsPerTask1;
	}
	else
	{
		start = (numRemainingItems * numItemsPerTask1) + ((taskId - numRemainingItems) * numItemsPerTask2);
		end   = start +  numItemsPerTask2;
	}

	*pstart = start;
	*pend = end;
}

#endif
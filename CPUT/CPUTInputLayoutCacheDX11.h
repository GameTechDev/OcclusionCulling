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
#ifndef __CPUTINPUTLAYOUTCACHERDX11_H__
#define __CPUTINPUTLAYOUTCACHERDX11_H__

#include "CPUTInputLayoutCache.h"
#include "CPUTOSServicesWin.h"
#include "CPUTVertexShaderDX11.h"
#include <D3D11.h> // D3D11_INPUT_ELEMENT_DESC
#include <map>

class CPUTInputLayoutCacheDX11:public CPUTInputLayoutCache
{
public:
    ~CPUTInputLayoutCacheDX11()
    {
        ClearLayoutCache();
    }
    static CPUTInputLayoutCacheDX11 *GetInputLayoutCache();
    static CPUTResult DeleteInputLayoutCache();
    CPUTResult GetLayout(ID3D11Device *pDevice, D3D11_INPUT_ELEMENT_DESC *pDXLayout, CPUTVertexShaderDX11 *pVertexShader, ID3D11InputLayout **ppInputLayout);
    void ClearLayoutCache();

private:
	struct LayoutKey
	{
		const D3D11_INPUT_ELEMENT_DESC *layout;
		void *vs;
		int nElems;
		bool layout_owned;

		LayoutKey();
		LayoutKey(const D3D11_INPUT_ELEMENT_DESC *pDXLayout, void *vs, bool just_ref);
		LayoutKey(const LayoutKey &x);
		~LayoutKey();

		LayoutKey& operator =(const LayoutKey &x);

		inline bool operator <(const LayoutKey &x) const
		{
			if (vs != x.vs)
				return vs < x.vs;

			if (nElems != x.nElems)
				return nElems < x.nElems;

			return memcmp(layout, x.layout, sizeof(*layout) * nElems) < 0;
		}
	};

    // singleton
    CPUTInputLayoutCacheDX11() { mLayoutList.clear(); }

    CPUTResult VerifyLayoutCompatibility(D3D11_INPUT_ELEMENT_DESC *pDXLayout, ID3DBlob *pVertexShaderBlob);

    static CPUTInputLayoutCacheDX11 *mpInputLayoutCache;
    std::map<LayoutKey, ID3D11InputLayout*> mLayoutList;
};

#endif //#define __CPUTINPUTLAYOUTCACHERDX11_H__
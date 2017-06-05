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

#include "CPUTTextureDX11.h"
#include "DDSTextureLoader.h"

// --- BEGIN dds.h

struct DDS_PIXELFORMAT
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwFourCC;
    DWORD dwRGBBitCount;
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwABitMask;
};

#define DDS_FOURCC 0x00000004  // DDPF_FOURCC
#define DDS_RGB    0x00000040  // DDPF_RGB
#define DDS_RGBA   0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS

const DDS_PIXELFORMAT DDSPF_DXT1 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','1'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT2 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','2'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT3 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','3'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT4 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','4'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_DXT5 =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('D','X','T','5'), 0, 0, 0, 0, 0 };

const DDS_PIXELFORMAT DDSPF_A8R8G8B8 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };

const DDS_PIXELFORMAT DDSPF_A1R5G5B5 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };

const DDS_PIXELFORMAT DDSPF_A4R4G4B4 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGBA, 0, 16, 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };

const DDS_PIXELFORMAT DDSPF_R8G8B8 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 24, 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };

const DDS_PIXELFORMAT DDSPF_R5G6B5 =
    { sizeof(DDS_PIXELFORMAT), DDS_RGB, 0, 16, 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };

#define DDS_HEADER_FLAGS_TEXTURE    0x00001007  // DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT 
#define DDS_HEADER_FLAGS_MIPMAP     0x00020000  // DDSD_MIPMAPCOUNT
#define DDS_HEADER_FLAGS_VOLUME     0x00800000  // DDSD_DEPTH
#define DDS_HEADER_FLAGS_PITCH      0x00000008  // DDSD_PITCH
#define DDS_HEADER_FLAGS_LINEARSIZE 0x00080000  // DDSD_LINEARSIZE

#define DDS_SURFACE_FLAGS_TEXTURE 0x00001000 // DDSCAPS_TEXTURE
#define DDS_SURFACE_FLAGS_MIPMAP  0x00400008 // DDSCAPS_COMPLEX | DDSCAPS_MIPMAP
#define DDS_SURFACE_FLAGS_CUBEMAP 0x00000008 // DDSCAPS_COMPLEX

#define DDS_CUBEMAP_POSITIVEX 0x00000600 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEX
#define DDS_CUBEMAP_NEGATIVEX 0x00000a00 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEX
#define DDS_CUBEMAP_POSITIVEY 0x00001200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEY
#define DDS_CUBEMAP_NEGATIVEY 0x00002200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEY
#define DDS_CUBEMAP_POSITIVEZ 0x00004200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_POSITIVEZ
#define DDS_CUBEMAP_NEGATIVEZ 0x00008200 // DDSCAPS2_CUBEMAP | DDSCAPS2_CUBEMAP_NEGATIVEZ

#define DDS_CUBEMAP_ALLFACES ( DDS_CUBEMAP_POSITIVEX | DDS_CUBEMAP_NEGATIVEX |\
                               DDS_CUBEMAP_POSITIVEY | DDS_CUBEMAP_NEGATIVEY |\
                               DDS_CUBEMAP_POSITIVEZ | DDS_CUBEMAP_NEGATIVEZ )

#define DDS_FLAGS_VOLUME 0x00200000 // DDSCAPS2_VOLUME


struct DDS_HEADER
{
    DWORD dwSize;
    DWORD dwHeaderFlags;
    DWORD dwHeight;
    DWORD dwWidth;
    DWORD dwPitchOrLinearSize;
    DWORD dwDepth; // only if DDS_HEADER_FLAGS_VOLUME is set in dwHeaderFlags
    DWORD dwMipMapCount;
    DWORD dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    DWORD dwSurfaceFlags;
    DWORD dwCubemapFlags;
    DWORD dwReserved2[3];
};

// ---- END dds.h

// TODO: Would be nice to find a better place for this decl.  But, not another file just for this.
const cString gDXGIFormatNames[] =
{
    _L("DXGI_FORMAT_UNKNOWN"),
    _L("DXGI_FORMAT_R32G32B32A32_TYPELESS"),
    _L("DXGI_FORMAT_R32G32B32A32_FLOAT"),
    _L("DXGI_FORMAT_R32G32B32A32_UINT"),
    _L("DXGI_FORMAT_R32G32B32A32_SINT"),
    _L("DXGI_FORMAT_R32G32B32_TYPELESS"),
    _L("DXGI_FORMAT_R32G32B32_FLOAT"),
    _L("DXGI_FORMAT_R32G32B32_UINT"),
    _L("DXGI_FORMAT_R32G32B32_SINT"),
    _L("DXGI_FORMAT_R16G16B16A16_TYPELESS"),
    _L("DXGI_FORMAT_R16G16B16A16_FLOAT"),
    _L("DXGI_FORMAT_R16G16B16A16_UNORM"),
    _L("DXGI_FORMAT_R16G16B16A16_UINT"),
    _L("DXGI_FORMAT_R16G16B16A16_SNORM"),
    _L("DXGI_FORMAT_R16G16B16A16_SINT"),
    _L("DXGI_FORMAT_R32G32_TYPELESS"),
    _L("DXGI_FORMAT_R32G32_FLOAT"),
    _L("DXGI_FORMAT_R32G32_UINT"),
    _L("DXGI_FORMAT_R32G32_SINT"),
    _L("DXGI_FORMAT_R32G8X24_TYPELESS"),
    _L("DXGI_FORMAT_D32_FLOAT_S8X24_UINT"),
    _L("DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS"),
    _L("DXGI_FORMAT_X32_TYPELESS_G8X24_UINT"),
    _L("DXGI_FORMAT_R10G10B10A2_TYPELESS"),
    _L("DXGI_FORMAT_R10G10B10A2_UNORM"),
    _L("DXGI_FORMAT_R10G10B10A2_UINT"),
    _L("DXGI_FORMAT_R11G11B10_FLOAT"),
    _L("DXGI_FORMAT_R8G8B8A8_TYPELESS"),
    _L("DXGI_FORMAT_R8G8B8A8_UNORM"),
    _L("DXGI_FORMAT_R8G8B8A8_UNORM_SRGB"),
    _L("DXGI_FORMAT_R8G8B8A8_UINT"),
    _L("DXGI_FORMAT_R8G8B8A8_SNORM"),
    _L("DXGI_FORMAT_R8G8B8A8_SINT"),
    _L("DXGI_FORMAT_R16G16_TYPELESS"),
    _L("DXGI_FORMAT_R16G16_FLOAT"),
    _L("DXGI_FORMAT_R16G16_UNORM"),
    _L("DXGI_FORMAT_R16G16_UINT"),
    _L("DXGI_FORMAT_R16G16_SNORM"),
    _L("DXGI_FORMAT_R16G16_SINT"),
    _L("DXGI_FORMAT_R32_TYPELESS"),
    _L("DXGI_FORMAT_D32_FLOAT"),
    _L("DXGI_FORMAT_R32_FLOAT"),
    _L("DXGI_FORMAT_R32_UINT"),
    _L("DXGI_FORMAT_R32_SINT"),
    _L("DXGI_FORMAT_R24G8_TYPELESS"),
    _L("DXGI_FORMAT_D24_UNORM_S8_UINT"),
    _L("DXGI_FORMAT_R24_UNORM_X8_TYPELESS"),
    _L("DXGI_FORMAT_X24_TYPELESS_G8_UINT"),
    _L("DXGI_FORMAT_R8G8_TYPELESS"),
    _L("DXGI_FORMAT_R8G8_UNORM"),
    _L("DXGI_FORMAT_R8G8_UINT"),
    _L("DXGI_FORMAT_R8G8_SNORM"),
    _L("DXGI_FORMAT_R8G8_SINT"),
    _L("DXGI_FORMAT_R16_TYPELESS"),
    _L("DXGI_FORMAT_R16_FLOAT"),
    _L("DXGI_FORMAT_D16_UNORM"),
    _L("DXGI_FORMAT_R16_UNORM"),
    _L("DXGI_FORMAT_R16_UINT"),
    _L("DXGI_FORMAT_R16_SNORM"),
    _L("DXGI_FORMAT_R16_SINT"),
    _L("DXGI_FORMAT_R8_TYPELESS"),
    _L("DXGI_FORMAT_R8_UNORM"),
    _L("DXGI_FORMAT_R8_UINT"),
    _L("DXGI_FORMAT_R8_SNORM"),
    _L("DXGI_FORMAT_R8_SINT"),
    _L("DXGI_FORMAT_A8_UNORM"),
    _L("DXGI_FORMAT_R1_UNORM"),
    _L("DXGI_FORMAT_R9G9B9E5_SHAREDEXP"),
    _L("DXGI_FORMAT_R8G8_B8G8_UNORM"),
    _L("DXGI_FORMAT_G8R8_G8B8_UNORM"),
    _L("DXGI_FORMAT_BC1_TYPELESS"),
    _L("DXGI_FORMAT_BC1_UNORM"),
    _L("DXGI_FORMAT_BC1_UNORM_SRGB"),
    _L("DXGI_FORMAT_BC2_TYPELESS"),
    _L("DXGI_FORMAT_BC2_UNORM"),
    _L("DXGI_FORMAT_BC2_UNORM_SRGB"),
    _L("DXGI_FORMAT_BC3_TYPELESS"),
    _L("DXGI_FORMAT_BC3_UNORM"),
    _L("DXGI_FORMAT_BC3_UNORM_SRGB"),
    _L("DXGI_FORMAT_BC4_TYPELESS"),
    _L("DXGI_FORMAT_BC4_UNORM"),
    _L("DXGI_FORMAT_BC4_SNORM"),
    _L("DXGI_FORMAT_BC5_TYPELESS"),
    _L("DXGI_FORMAT_BC5_UNORM"),
    _L("DXGI_FORMAT_BC5_SNORM"),
    _L("DXGI_FORMAT_B5G6R5_UNORM"),
    _L("DXGI_FORMAT_B5G5R5A1_UNORM"),
    _L("DXGI_FORMAT_B8G8R8A8_UNORM"),
    _L("DXGI_FORMAT_B8G8R8X8_UNORM"),
    _L("DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM"),
    _L("DXGI_FORMAT_B8G8R8A8_TYPELESS"),
    _L("DXGI_FORMAT_B8G8R8A8_UNORM_SRGB"),
    _L("DXGI_FORMAT_B8G8R8X8_TYPELESS"),
    _L("DXGI_FORMAT_B8G8R8X8_UNORM_SRGB"),
    _L("DXGI_FORMAT_BC6H_TYPELESS"),
    _L("DXGI_FORMAT_BC6H_UF16"),
    _L("DXGI_FORMAT_BC6H_SF16"),
    _L("DXGI_FORMAT_BC7_TYPELESS"),
    _L("DXGI_FORMAT_BC7_UNORM"),
    _L("DXGI_FORMAT_BC7_UNORM_SRGB")
};
const cString *gpDXGIFormatNames = gDXGIFormatNames;

//-----------------------------------------------------------------------------
CPUTTexture *CPUTTextureDX11::CreateTexture( const cString &name, const cString &absolutePathAndFilename, bool loadAsSRGB )
{
    // TODO:  Delegate to derived class.  We don't currently have CPUTTextureDX11
    ID3D11ShaderResourceView *pShaderResourceView = NULL;
    ID3D11Resource *pTexture = NULL;
    ID3D11Device *pD3dDevice= CPUT_DX11::GetDevice();
    CPUTResult result = CreateNativeTexture( pD3dDevice, absolutePathAndFilename, &pShaderResourceView, &pTexture, loadAsSRGB );
    ASSERT( CPUTSUCCESS(result), _L("Error loading texture: '")+absolutePathAndFilename );

    CPUTTextureDX11 *pNewTexture = new CPUTTextureDX11();
    pNewTexture->mName = name;
    pNewTexture->SetTextureAndShaderResourceView( pTexture, pShaderResourceView );
    pTexture->Release();
    pShaderResourceView->Release();

    CPUTAssetLibrary::GetAssetLibrary()->AddTexture( absolutePathAndFilename, pNewTexture);

    return pNewTexture;
}

//-----------------------------------------------------------------------------

class MappedFile
{
	HANDLE hfile;
	HANDLE hmapping;
	void *view;

public:
	MappedFile() : hfile(INVALID_HANDLE_VALUE), hmapping(NULL), view(0) {}
	~MappedFile()
	{
		Close();
	}

	void *Open(LPCWSTR filename)
	{
		Close();

		if ((hfile = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0)) == INVALID_HANDLE_VALUE ||
			(hmapping = CreateFileMapping(hfile, 0, PAGE_READONLY, 0, 0, NULL)) == NULL ||
			(view = MapViewOfFile(hmapping, FILE_MAP_READ, 0, 0, 0)) == NULL)
			Close();

		return view;
	}

	void Close()
	{
		if (view != 0)
		{
			UnmapViewOfFile(view);
			view = 0;
		}

		if (hmapping != NULL)
		{
			CloseHandle(hmapping);
			hmapping = NULL;
		}

		if (hfile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hfile);
			hfile = INVALID_HANDLE_VALUE;
		}
	}
};

CPUTResult CPUTTextureDX11::CreateNativeTexture(
    ID3D11Device *pD3dDevice,
    const cString &fileName,
    ID3D11ShaderResourceView **ppShaderResourceView,
    ID3D11Resource **ppTexture,
    bool ForceLoadAsSRGB
){
    HRESULT hr;

    hr = DirectX::CreateDDSTextureFromFileEx(
        pD3dDevice,
        fileName.c_str(),
        0,//maxsize
        D3D11_USAGE_DEFAULT,
        D3D11_BIND_SHADER_RESOURCE,
        0,
        0,
        ForceLoadAsSRGB,
        ppTexture,
        ppShaderResourceView);

    ASSERT( SUCCEEDED(hr), _L("Failed to load texture: ") + fileName );
    CPUTSetDebugName( *ppTexture, fileName );

    ASSERT( SUCCEEDED(hr), _L("Failed to create texture shader resource view.") );
    CPUTSetDebugName( *ppShaderResourceView, fileName );

    return CPUT_SUCCESS;
}

//-----------------------------------------------------------------------------
CPUTResult CPUTTextureDX11::GetSRGBEquivalent(DXGI_FORMAT inFormat, DXGI_FORMAT& sRGBFormat)
{
    switch( inFormat )
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            sRGBFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
            return CPUT_SUCCESS;
        case DXGI_FORMAT_B8G8R8X8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            sRGBFormat = DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
            return CPUT_SUCCESS;
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
            sRGBFormat = DXGI_FORMAT_BC1_UNORM_SRGB;
            return CPUT_SUCCESS;
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
            sRGBFormat = DXGI_FORMAT_BC2_UNORM_SRGB;
            return CPUT_SUCCESS;
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
            sRGBFormat = DXGI_FORMAT_BC3_UNORM_SRGB;
            return CPUT_SUCCESS;
        case DXGI_FORMAT_BC7_UNORM:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            sRGBFormat = DXGI_FORMAT_BC7_UNORM_SRGB;
            return CPUT_SUCCESS;
    };
    return CPUT_ERROR_UNSUPPORTED_SRGB_IMAGE_FORMAT;
}

// This function returns the DXGI string equivalent of the DXGI format for
// error reporting/display purposes
//-----------------------------------------------------------------------------
const cString &CPUTTextureDX11::GetDXGIFormatString(DXGI_FORMAT format)
{
    ASSERT( (format>=0) && (format<=DXGI_FORMAT_BC7_UNORM_SRGB), _L("Invalid DXGI Format.") );
    return gpDXGIFormatNames[format];
}

// Given a certain DXGI texture format, does it even have an equivalent sRGB one
//-----------------------------------------------------------------------------
bool CPUTTextureDX11::DoesExistEquivalentSRGBFormat(DXGI_FORMAT inFormat)
{
    DXGI_FORMAT outFormat;

    if( CPUT_ERROR_UNSUPPORTED_SRGB_IMAGE_FORMAT == GetSRGBEquivalent(inFormat, outFormat) )
    {
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
D3D11_MAPPED_SUBRESOURCE CPUTTextureDX11::MapTexture( CPUTRenderParameters &params, eCPUTMapType type, bool wait )
{
    // Mapping for DISCARD requires dynamic buffer.  Create dynamic copy?
    // Could easily provide input flag.  But, where would we specify? Don't like specifying in the .set file
    // Because mapping is something the application wants to do - it isn't inherent in the data.
    // Could do Clone() and pass dynamic flag to that.
    // But, then we have two.  Could always delete the other.
    // Could support programatic flag - apply to all loaded models in the .set
    // Could support programatic flag on model.  Load model first, then load set.
    // For now, simply support CopyResource mechanism.
    HRESULT hr;
    ID3D11Device *pD3dDevice = CPUT_DX11::GetDevice();
    CPUTRenderParametersDX *pParamsDX11 = (CPUTRenderParametersDX*)&params;
    ID3D11DeviceContext *pContext = pParamsDX11->mpContext;

    if( !mpTextureStaging )
    {
        // Annoying.  We need to create the texture differently, based on dimension.
        D3D11_RESOURCE_DIMENSION dimension;
        mpTexture->GetType(&dimension);
        switch( dimension )
        {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                D3D11_TEXTURE1D_DESC desc;
                ((ID3D11Texture1D*)mpTexture)->GetDesc( &desc );
                desc.Usage = D3D11_USAGE_STAGING;
                switch( type )
                {
                case CPUT_MAP_READ:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_READ_WRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_WRITE:
                case CPUT_MAP_WRITE_DISCARD:
                case CPUT_MAP_NO_OVERWRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                };
                hr = pD3dDevice->CreateTexture1D( &desc, NULL, (ID3D11Texture1D**)&mpTextureStaging );
                ASSERT( SUCCEEDED(hr), _L("Failed to create staging texture") );
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                D3D11_TEXTURE2D_DESC desc;
                ((ID3D11Texture2D*)mpTexture)->GetDesc( &desc );
                desc.Usage = D3D11_USAGE_STAGING;
                switch( type )
                {
                case CPUT_MAP_READ:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_READ_WRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_WRITE:
                case CPUT_MAP_WRITE_DISCARD:
                case CPUT_MAP_NO_OVERWRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                };
                hr = pD3dDevice->CreateTexture2D( &desc, NULL, (ID3D11Texture2D**)&mpTextureStaging );
                ASSERT( SUCCEEDED(hr), _L("Failed to create staging texture") );
                break;
            }
        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                D3D11_TEXTURE3D_DESC desc;
                ((ID3D11Texture3D*)mpTexture)->GetDesc( &desc );
                desc.Usage = D3D11_USAGE_STAGING;
                switch( type )
                {
                case CPUT_MAP_READ:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_READ_WRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                case CPUT_MAP_WRITE:
                case CPUT_MAP_WRITE_DISCARD:
                case CPUT_MAP_NO_OVERWRITE:
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                    desc.BindFlags = 0;
                    break;
                };
                hr = pD3dDevice->CreateTexture3D( &desc, NULL, (ID3D11Texture3D**)&mpTextureStaging );
                ASSERT( SUCCEEDED(hr), _L("Failed to create staging texture") );
                break;
            }
        default:
            ASSERT(0, _L("Unkown texture dimension") );
            break;
        }
    }
    else
    {
        ASSERT( mMappedType == type, _L("Mapping with a different CPU access than creation parameter.") );
    }
    D3D11_MAPPED_SUBRESOURCE info;
    switch( type )
    {
    case CPUT_MAP_READ:
    case CPUT_MAP_READ_WRITE:
        // TODO: Copying and immediately mapping probably introduces a stall.
        // Expose the copy externally?
        // TODO: copy only if changed?
        // Copy only first time?
        // Copy the GPU version before we read from it.
        pContext->CopyResource( mpTextureStaging, mpTexture );
        break;
    };
    hr = pContext->Map( mpTextureStaging, wait ? 0 : D3D11_MAP_FLAG_DO_NOT_WAIT, (D3D11_MAP)type, 0, &info );
    mMappedType = type;
    return info;
} // CPUTTextureDX11::Map()

//-----------------------------------------------------------------------------
void CPUTTextureDX11::UnmapTexture( CPUTRenderParameters &params )
{
    ASSERT( mMappedType != CPUT_MAP_UNDEFINED, _L("Can't unmap a render target that isn't mapped.") );

    CPUTRenderParametersDX *pParamsDX11 = (CPUTRenderParametersDX*)&params;
    ID3D11DeviceContext *pContext = pParamsDX11->mpContext;

    pContext->Unmap( mpTextureStaging, 0 );

    // If we were mapped for write, then copy staging buffer to GPU
    switch( mMappedType )
    {
    case CPUT_MAP_READ:
        break;
    case CPUT_MAP_READ_WRITE:
    case CPUT_MAP_WRITE:
    case CPUT_MAP_WRITE_DISCARD:
    case CPUT_MAP_NO_OVERWRITE:
        pContext->CopyResource( mpTexture, mpTextureStaging );
        break;
    };
} // CPUTTextureDX11::Unmap()


//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
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

// ********************************************************************************************************
struct VS_INPUT
{
    float3 Pos      : POSITION; // Projected position
};
struct PS_INPUT
{
    float4 Pos      : SV_POSITION;
};

// ********************************************************************************************************
cbuffer cbPerModelValues
{
    row_major float4x4 World : WORLD;
    row_major float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
    row_major float4x4 InverseWorld : INVERSEWORLD;
              float4   LightDirection;
              float4   EyePosition;
    row_major float4x4 LightWorldViewProjection;
};

// ********************************************************************************************************
// TODO: Note: nothing sets these values yet
cbuffer cbPerFrameValues
{
    row_major float4x4  View;
    row_major float4x4  Projection;
};

// ********************************************************************************************************
PS_INPUT VSMain( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( float4( input.Pos, 1.0f), WorldViewProjection );
    output.Pos.z -= 0.0001 * output.Pos.w;
    return output;
}

// ********************************************************************************************************
float4 PSMain( PS_INPUT input ) : SV_Target
{
    // TODO: Don't have pixel shader.
    return float4(1,0,0,1);
}

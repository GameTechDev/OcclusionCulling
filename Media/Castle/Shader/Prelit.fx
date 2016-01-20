// ********************************************************************************************************    
struct VS_INPUT    
{    
    float3 Pos      : POSITION; // Projected position    
    float3 Norm     : NORMAL;    
    float2 Uv       : TEXCOORD0;    
};    
struct PS_INPUT    
{    
    float4 Pos      : SV_POSITION;    
    float3 Norm     : NORMAL;    
    float2 Uv       : TEXCOORD0;    
    float4 LightUv  : TEXCOORD1;    
    float3 Position : TEXCOORD2; // Object space position     
};    
// ********************************************************************************************************    
    Texture2D    TEXTURE0 : register( t0 );    
    SamplerState SAMPLER0 : register( s0 );    
    Texture2D    _Shadow  : register( t1 );    
    SamplerComparisonState SAMPLER1 : register( s1 );    
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
    output.Pos      = mul( float4( input.Pos, 1.0f), WorldViewProjection );    
    output.Position = mul( float4( input.Pos, 1.0f), World ).xyz;    
    // TODO: transform the light into object space instead of the normal into world space    
    output.Norm = mul( input.Norm, (float3x3)World );    
    output.Uv   = float2(input.Uv.x, input.Uv.y);    
    output.LightUv   = mul( float4( input.Pos, 1.0f), LightWorldViewProjection );    
    return output;    
}    
// ********************************************************************************************************    
float4 PSMain( PS_INPUT input ) : SV_Target    
{    
    float4 diffuseTexture = TEXTURE0.Sample( SAMPLER0, input.Uv );    
    return diffuseTexture;
}    

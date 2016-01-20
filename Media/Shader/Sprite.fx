
// ********************************************************************************************************
struct VS_INPUT
{
    float3 Pos     : POSITION;
    float2 Uv      : TEXCOORD0;
};
struct PS_INPUT
{
    float4 Pos     : SV_POSITION;
    float2 Uv      : TEXCOORD0;
};
// ********************************************************************************************************
Texture2D    TEXTURE0 : register( t0 );
SamplerState SAMPLER0 : register( s0 );

// ********************************************************************************************************
PS_INPUT VSMain( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos  = float4( input.Pos, 1.0f);
    output.Uv   = input.Uv;
    return output;
}
// ********************************************************************************************************
float4 PSMain( PS_INPUT input ) : SV_Target
{
    float4 diffuseTexture = TEXTURE0.Sample( SAMPLER0, input.Uv);
    return pow(diffuseTexture, 0.2f);
}

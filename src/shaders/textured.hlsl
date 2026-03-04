cbuffer MVP : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

struct VSInput
{
    float3 pos : POSITION;
    float3 norm : NORMAL;
    float2 uv : TEXCOORD;
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD;
};

VSOutput VSMain(VSInput input)
{
    VSOutput o;
    
    float4 worldPos = mul(float4(input.pos, 1.0f), model);
    float4 viewPos = mul(worldPos, view);
    
    o.pos = mul(viewPos, projection);
    o.uv = input.uv;
    return o;
}

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 PSMain(VSOutput input) : SV_Target
{
    return tex.Sample(samp, input.uv);
}
cbuffer CB : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
    float3 cameraPos;
    float isSelected;
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
    
    float outlineWidth = 0.05f;
    float3 offset = input.norm * (outlineWidth * isSelected);
    float3 finalPos = input.pos + offset;

    float4 worldPos = mul(float4(finalPos, 1.0f), model);
    o.pos = mul(mul(worldPos, view), projection);
    o.uv = input.uv;
    
    return o;
}

Texture2D tex : register(t0);
SamplerState samp : register(s0);

float4 PSMain(VSOutput input) : SV_Target
{
    float4 texColor = tex.Sample(samp, input.uv);
    float4 outlineColor = float4(1.0, 1.0, 1.0, 1.0);
    return lerp(texColor, outlineColor, isSelected);
}
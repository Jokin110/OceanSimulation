
cbuffer PerObjectBuffer : register(b0)
{
    row_major matrix m_WorldMatrix;
    row_major matrix m_ViewProjectionMatrix;
}

struct VSInput
{
    float3 m_Position : POSITION;
    float2 m_UV : TEXCOORD0;
    nointerpolation int m_TextureIndex : TEXCOORD1;
};

struct VSOutput
{
    float2 m_UV : TEXCOORD0;
    float3 m_WorldPosition : TEXCOORD1;
    int m_TextureIndex : TEXCOORD2;
    float4 m_Position : SV_POSITION;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    float3 worldPosition = mul(float4(input.m_Position, 1.0), m_WorldMatrix).xyz;

    output.m_UV = input.m_UV;
    output.m_WorldPosition = worldPosition;
    output.m_TextureIndex = input.m_TextureIndex;
    output.m_Position = mul(float4(worldPosition, 1.0), m_ViewProjectionMatrix);
    output.m_Position.z = output.m_Position.w - 0.0001f; // Push the skybox slightly back to prevent z-fighting with the ocean surface

    return output;
}
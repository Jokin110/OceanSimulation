
cbuffer PerObjectBuffer : register(b0)
{
    row_major matrix m_WorldMatrix;
    row_major matrix m_ViewProjectionMatrix;
}

struct VSInput
{
    float3 m_Position : POSITION;
};

struct VSOutput
{
    float3 m_LocalPosition : TEXCOORD0;
    float3 m_WorldPosition : TEXCOORD1;
    float4 m_Position : SV_POSITION;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    output.m_LocalPosition = input.m_Position;
    output.m_WorldPosition = mul(float4(input.m_Position, 1.0), m_WorldMatrix);
    output.m_Position = mul(float4(output.m_WorldPosition, 1.0), m_ViewProjectionMatrix);
    output.m_Position.z = output.m_Position.w - 0.001f; // Push the skybox slightly back to prevent z-fighting with the ocean surface

    return output;
}

struct VSInput
{
    float2 m_UV : TEXCOORD0;
};

struct VSOutput
{
    float2 m_UV : TEXCOORD0;
    float4 m_Position : SV_POSITION;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    output.m_UV = input.m_UV;
    output.m_Position.x = (input.m_UV.x - 0.5f) * 2.0f;
    output.m_Position.y = (1.0f - input.m_UV.y - 0.5f) * 2.0f;
    output.m_Position.zw = float2(0.0f, 1.0f);
    
    return output;
}

struct PSInput
{
    float2 m_UV : TEXCOORD0;
    float4 m_Position : SV_POSITION;
};

Texture2D SceneColorTexture : register(t0);

SamplerState PointSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    return SceneColorTexture.Sample(PointSampler, input.m_UV);
}
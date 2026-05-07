
struct PSInput
{
    float3 m_LocalPosition : TEXCOORD0;
    float4 m_Position : SV_POSITION;
};

Texture2D SkyBoxTexture[6] : register(t0);
SamplerState LinearSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float4 skyColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    return skyColor;
}
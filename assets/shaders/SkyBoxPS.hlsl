
struct PSInput
{
    float2 m_UV : TEXCOORD0;
    float3 m_WorldPosition : TEXCOORD1;
    nointerpolation int m_TextureIndex : TEXCOORD2;
    float4 m_Position : SV_POSITION;
};

Texture2D SkyBoxTexture[6] : register(t0);
SamplerState LinearSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float4 skyColor = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    switch (input.m_TextureIndex)
    {
        case 0:
            skyColor = SkyBoxTexture[0].Sample(LinearSampler, input.m_UV);
            break;
        case 1:
            skyColor = SkyBoxTexture[1].Sample(LinearSampler, input.m_UV);
            break;
        case 2:
            skyColor = SkyBoxTexture[2].Sample(LinearSampler, input.m_UV);
            break;
        case 3:
            skyColor = SkyBoxTexture[3].Sample(LinearSampler, input.m_UV);
            break;
        case 4:
            skyColor = SkyBoxTexture[4].Sample(LinearSampler, input.m_UV);
            break;
        case 5:
            skyColor = SkyBoxTexture[5].Sample(LinearSampler, input.m_UV);
            break;
    }
    
    return skyColor;
}
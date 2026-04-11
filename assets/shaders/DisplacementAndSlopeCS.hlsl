
// Displacement and slope input textures
Texture2D<float4> XYDisplacementTexture : register(t0);
Texture2D<float4> ZDisplacementXXDerivativeTexture : register(t1);
Texture2D<float4> XZYXDerivativeTexture : register(t2);
Texture2D<float4> YZZZDerivativeTexture : register(t3);

// Displacement and slope output textures
RWTexture2D<float4> DisplacementTexture : register(u0);
RWTexture2D<float4> SlopeTexture : register(u1);

cbuffer DisplacementAndSlopeParams : register(b0)
{
    float m_FoamBias;
    float m_DecayFactor;
    float m_DeltaTime;
    float Padding;
};

[numthreads(16, 16, 1)]
void Main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint x = dispatchThreadID.x;
    uint y = dispatchThreadID.y;
    
    uint2 uv = uint2(x, y);
    
    float4 xyDisplacement = XYDisplacementTexture[uv];
    float4 zDisplacementXXDerivative = ZDisplacementXXDerivativeTexture[uv];
    float4 xzyxDerivative = XZYXDerivativeTexture[uv];
    float4 yzzzDerivative = YZZZDerivativeTexture[uv];
    
    float dxx = zDisplacementXXDerivative.b;
    float dxz = xzyxDerivative.r;
    float dyx = xzyxDerivative.b;
    float dyz = yzzzDerivative.r;
    float dzz = yzzzDerivative.b;
    
    float3 tangent = normalize(float3(1.0f + dxx, dyx, dxz));
    float3 binormal = normalize(float3(dxz, dyz, 1.0f + dzz));
    
    float3 normal = normalize(cross(binormal, tangent));
    
    float Jxx = 1.0f + dxx;
    float Jzz = 1.0f + dzz;
    float Jxz = dxz;
    
    float jacobian = Jxx * Jzz - Jxz * Jxz;
    
    float foam = saturate(SlopeTexture[uv].a * exp(-m_DecayFactor * m_DeltaTime));
    foam += jacobian < m_FoamBias ? 1.0f : 0.0f;
    foam = saturate(foam);
    
    DisplacementTexture[uv] = float4(xyDisplacement.r, xyDisplacement.b, zDisplacementXXDerivative.r, 0.0f);
    SlopeTexture[uv] = float4(normal, foam);
}
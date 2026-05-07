#define PI 3.14159265359

RWTexture2D<float4> PrecomputeBuffer : register(u0);
Texture2D<float4> PrecomputedData : register(t0);
RWTexture2D<float4> Buffer0 : register(u1);
RWTexture2D<float4> Buffer1 : register(u2);

cbuffer FFTParams : register(b0)
{
    uint Size;
    uint Step;
    uint PingPong; // Use uint for bool to align properly
    uint Padding;
};

float2 ComplexMult(float2 a, float2 b)
{
    return float2(a.r * b.r - a.g * b.g, a.r * b.g + a.g * b.r);
}

float2 ComplexExp(float2 a)
{
    return float2(cos(a.y), sin(a.y)) * exp(a.x);
}

[numthreads(1, 8, 1)]
void PrecomputeTwiddleFactorsAndInputIndices(uint3 id : SV_DispatchThreadID)
{
    uint b = Size >> (id.x + 1);
    float2 mult = 2 * PI * float2(0, 1) / Size;
    uint i = (2 * b * (id.y / b) + id.y % b) % Size;
    float2 twiddle = ComplexExp(-mult * ((id.y / b) * b));
    
    PrecomputeBuffer[id.xy] = float4(twiddle.x, twiddle.y, i, i + b);
    PrecomputeBuffer[uint2(id.x, id.y + Size / 2)] = float4(-twiddle.x, -twiddle.y, i, i + b);
}

[numthreads(8, 8, 1)]
void HorizontalStepFFT(uint3 id : SV_DispatchThreadID)
{
    float4 data = PrecomputedData[uint2(Step, id.x)];
    uint2 inputsIndices = (uint2) data.ba;
    
    if (PingPong)
    {
        float4 valA = Buffer0[uint2(inputsIndices.x, id.y)];
        float4 valB = Buffer0[uint2(inputsIndices.y, id.y)];
        
        Buffer1[id.xy] = float4(
            valA.rg + ComplexMult(data.rg, valB.rg),
            valA.ba + ComplexMult(data.rg, valB.ba)
        );
    }
    else
    {
        float4 valA = Buffer1[uint2(inputsIndices.x, id.y)];
        float4 valB = Buffer1[uint2(inputsIndices.y, id.y)];
        
        Buffer0[id.xy] = float4(
            valA.rg + ComplexMult(data.rg, valB.rg),
            valA.ba + ComplexMult(data.rg, valB.ba)
        );
    }
}

[numthreads(8, 8, 1)]
void VerticalStepFFT(uint3 id : SV_DispatchThreadID)
{
    float4 data = PrecomputedData[uint2(Step, id.y)];
    uint2 inputsIndices = (uint2) data.ba;
    
    if (PingPong)
    {
        float4 valA = Buffer0[uint2(id.x, inputsIndices.x)];
        float4 valB = Buffer0[uint2(id.x, inputsIndices.y)];
        
        Buffer1[id.xy] = float4(
            valA.rg + ComplexMult(data.rg, valB.rg),
            valA.ba + ComplexMult(data.rg, valB.ba)
        );
    }
    else
    {
        float4 valA = Buffer1[uint2(id.x, inputsIndices.x)];
        float4 valB = Buffer1[uint2(id.x, inputsIndices.y)];
        
        Buffer0[id.xy] = float4(
            valA.rg + ComplexMult(data.rg, valB.rg),
            valA.ba + ComplexMult(data.rg, valB.ba)
        );
    }
}

[numthreads(8, 8, 1)]
void HorizontalStepInverseFFT(uint3 id : SV_DispatchThreadID)
{
    float4 data = PrecomputedData[uint2(Step, id.x)];
    uint2 inputsIndices = (uint2) data.ba;
    float2 twiddle = float2(data.r, -data.g);
    
    if (PingPong)
    {
        float4 valA = Buffer0[uint2(inputsIndices.x, id.y)];
        float4 valB = Buffer0[uint2(inputsIndices.y, id.y)];
        
        Buffer1[id.xy] = float4(
            valA.rg + ComplexMult(twiddle, valB.rg),
            valA.ba + ComplexMult(twiddle, valB.ba)
        );
    }
    else
    {
        float4 valA = Buffer1[uint2(inputsIndices.x, id.y)];
        float4 valB = Buffer1[uint2(inputsIndices.y, id.y)];
        
        Buffer0[id.xy] = float4(
            valA.rg + ComplexMult(twiddle, valB.rg),
            valA.ba + ComplexMult(twiddle, valB.ba)
        );
    }
}

[numthreads(8, 8, 1)]
void VerticalStepInverseFFT(uint3 id : SV_DispatchThreadID)
{
    float4 data = PrecomputedData[uint2(Step, id.y)];
    uint2 inputsIndices = (uint2) data.ba;
    float2 twiddle = float2(data.r, -data.g);
    
    if (PingPong)
    {
        float4 valA = Buffer0[uint2(id.x, inputsIndices.x)];
        float4 valB = Buffer0[uint2(id.x, inputsIndices.y)];
        
        Buffer1[id.xy] = float4(
            valA.rg + ComplexMult(twiddle, valB.rg),
            valA.ba + ComplexMult(twiddle, valB.ba)
        );
    }
    else
    {
        float4 valA = Buffer1[uint2(id.x, inputsIndices.x)];
        float4 valB = Buffer1[uint2(id.x, inputsIndices.y)];
        
        Buffer0[id.xy] = float4(
            valA.rg + ComplexMult(twiddle, valB.rg),
            valA.ba + ComplexMult(twiddle, valB.ba)
        );
    }
}

[numthreads(8, 8, 1)]
void Scale(uint3 id : SV_DispatchThreadID)
{
    // Cast to float to ensure proper floating-point division across all 4 channels
    Buffer0[id.xy] = Buffer0[id.xy] / (float) (Size * Size);
}

[numthreads(8, 8, 1)]
void Permute(uint3 id : SV_DispatchThreadID)
{
    // This scalar value automatically broadcasts to the r, g, b, and a channels
    Buffer0[id.xy] = Buffer0[id.xy] * (1.0 - 2.0 * ((id.x + id.y) % 2));
}
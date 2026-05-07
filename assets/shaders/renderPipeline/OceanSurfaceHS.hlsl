struct HSInput
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct HSOutput
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

cbuffer PerObjectBuffer : register(b0)
{
    row_major matrix m_WorldMatrix;
    
    float3 m_CameraPosition;
    
    float m_MinDistance;
    float m_MaxDistance;
    int m_TessFactorExponent;
    
    float2 m_Padding;
}

float CalculateTessFactor(float3 v0, float3 v1)
{
    float3 edgeCenterLocal = (v0 + v1) * 0.5f;
    
    float3 edgeCenterWorld = mul(float4(edgeCenterLocal, 1), m_WorldMatrix).xyz;
    
    float distanceToCamera = length(m_CameraPosition - edgeCenterWorld);
    
    return max(1.0f, 64.0f * pow(saturate((m_MaxDistance - distanceToCamera) / (m_MaxDistance - m_MinDistance)), m_TessFactorExponent));
}

PatchTess ConstantHS(InputPatch<HSInput, 4> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;
    
    float3 centerLocal = 0.25f * (patch[0].Position + patch[1].Position + patch[2].Position + patch[3].Position);
    
    float3 v0 = patch[0].Position; // bottomLeftVertex
    float3 v1 = patch[1].Position; // topLeftVertex
    float3 v2 = patch[2].Position; // bottomRightVertex
    float3 v3 = patch[3].Position; // topRightVertex
    
    pt.EdgeTess[0] = CalculateTessFactor(v0, v1); // Left edge
    pt.EdgeTess[1] = CalculateTessFactor(v1, v3); // Top edge
    pt.EdgeTess[2] = CalculateTessFactor(v2, v3); // Right edge
    pt.EdgeTess[3] = CalculateTessFactor(v0, v2); // Bottom edge
    
    pt.InsideTess[0] = CalculateTessFactor(centerLocal, centerLocal); // Horizontal inside
    pt.InsideTess[1] = CalculateTessFactor(centerLocal, centerLocal); // Vertical inside

    return pt;
}

// --- MAIN HULL SHADER ---
[domain("quad")]
[partitioning("fractional_even")] // Can be integer, fractional_even, or fractional_odd
[outputtopology("triangle_cw")] // Generate clockwise triangles inside the quad
[outputcontrolpoints(4)] // We output 4 control points per patch
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HSOutput Main(
    InputPatch<HSInput, 4> p,
    uint i : SV_OutputControlPointID,
    uint patchID : SV_PrimitiveID)
{
    HSOutput output = (HSOutput) 0;
    
    output.Position = p[i].Position;
    output.Color = p[i].Color;

    return output;
}
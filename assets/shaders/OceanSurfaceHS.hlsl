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
    row_major matrix m_InverseTransposeWorldMatrix;
    row_major matrix m_ViewProjectionMatrix;
    
    float3 m_CameraPosition;
    
    int m_OceanTextureSize; // Size of the ocean texture (e.g., 256x256)
    float m_PatchSize[4]; // Size of the ocean patch in world units
}

PatchTess ConstantHS(InputPatch<HSInput, 4> patch, uint patchID : SV_PrimitiveID)
{
    PatchTess pt;

    // --- DYNAMIC LOD LOGIC ---
    // Here you would calculate the distance from the CameraPosition to the patch.
    // For now, we will hardcode a tessellation factor.
    // Max factor in D3D11 is 64.0.
    float3 centerLocal = 0.25f * (patch[0].Position + patch[1].Position + patch[2].Position + patch[3].Position);
    float3 centerWorld = mul(float4(centerLocal, 1), m_WorldMatrix).xyz;
    float distanceToCamera = length(m_CameraPosition - centerWorld);
    
    const float maxDistance = 1200.0f; // Beyond this distance, use minimum tessellation
    const float minDistance = 30.0f; // Within this distance, use maximum tessellation
    
    float tessFactor = max(1.0f, 64.0f * pow(saturate((maxDistance - distanceToCamera) / (maxDistance - minDistance)), 16));
    //tessFactor = 1;
    
    pt.EdgeTess[0] = tessFactor; // Left edge
    pt.EdgeTess[1] = tessFactor; // Top edge
    pt.EdgeTess[2] = tessFactor; // Right edge
    pt.EdgeTess[3] = tessFactor; // Bottom edge
    
    pt.InsideTess[0] = tessFactor; // Horizontal inside
    pt.InsideTess[1] = tessFactor; // Vertical inside

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
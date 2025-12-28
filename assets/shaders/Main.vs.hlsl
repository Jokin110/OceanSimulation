
cbuffer PerObjectBuffer : register(b0)
{
    row_major matrix m_WorldMatrix;
    row_major matrix m_InverseTransposeWorldMatrix;
    row_major matrix m_ViewProjectionMatrix;
    
    float m_Time;
    float3 m_CameraPosition;
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 color : COLOR;
};

struct VSOutput
{
    float3 worldPosition : TEXCOORD0;
    float3 normalWS : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float3 color : COLOR;
    float4 position : SV_POSITION;
};

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    float3 worldPosition = mul(float4(input.position, 1.0), m_WorldMatrix).xyz;
    
    // Wave 1
    float amplitude = 0.5f;
    float2 direction = normalize(float2(1.0, 0.1)); 
    float frequency = 0.3f; 
    float speed = 0.5f; 

    float height = amplitude * sin(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    
    float dx = amplitude * direction.x * frequency * cos(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    float dz = amplitude * direction.y * frequency * cos(dot(direction, worldPosition.xz) * frequency + m_Time * speed);

    // Wave 2
    amplitude = 0.25f;
    direction = normalize(float2(0.7, 0.7));
    frequency = 0.65f;
    speed = 1.0f;

    height += amplitude * sin(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    
    dx += amplitude * direction.x * frequency * cos(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    dz += amplitude * direction.y * frequency * cos(dot(direction, worldPosition.xz) * frequency + m_Time * speed);

    //  Wave 3
    amplitude = 0.12f;
    direction = normalize(float2(-0.2, 0.9));
    frequency = 1.8f;
    speed = 1.8f;

    height += amplitude * sin(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    
    dx += amplitude * direction.x * frequency * cos(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    dz += amplitude * direction.y * frequency * cos(dot(direction, worldPosition.xz) * frequency + m_Time * speed);

    // Wave 4
    amplitude = 0.04f;
    direction = normalize(float2(0.4, -0.6));
    frequency = 4.5f;
    speed = 2.4f;
    
    height += amplitude * sin(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    
    dx += amplitude * direction.x * frequency * cos(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    dz += amplitude * direction.y * frequency * cos(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    
    worldPosition.y += height;
    
    float3 normal = normalize(float3(-dx, 1.0f, -dz));
    
    output.worldPosition = worldPosition;
    output.normalWS = normalize(mul(float4(normal, 0.0), m_InverseTransposeWorldMatrix).xyz);
    output.viewVector = normalize(m_CameraPosition - worldPosition);
    output.color = input.color;
    output.position = mul(float4(worldPosition, 1.0), m_ViewProjectionMatrix);
    
    return output;
}
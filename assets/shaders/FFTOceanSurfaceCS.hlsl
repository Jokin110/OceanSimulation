
// Input: The initial spectrum created at start up
//Texture2D<float4> H0Texture : register(t0);

// Output: The updated spectrum for this specific frame
RWTexture2D<float4> displacementTexture : register(u0);

// Define the Thread Group size
[numthreads(16, 16, 1)]
void Main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint x = dispatchThreadID.x;
    uint y = dispatchThreadID.y;
}
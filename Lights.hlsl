#include "ShaderStructs.hlsli"

#define MAX_SPECULAR_EXPONENT 256.0f

#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

// Defines a single light that can be sent to the GPU
// Note: This must match light struct in shaders
//       and must also be a multiple of 16 bytes!
struct Light
{
    int Type;
    float3 Direction; // 16 bytes

    float Range;
    float3 Position; // 32 bytes

    float Intensity;
    float3 Color; // 48 bytes

    float SpotInnerAngle;
    float SpotOuterAngle;
    float3 Padding; // 64 bytes
};

float4 main(VertexToPixel input) : SV_TARGET
{
    return float4(1, 1, 0, 1); // Return a white color (R, G, B, A)
}

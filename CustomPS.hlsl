
#include "ShaderStructs.hlsli"

cbuffer ExternalData : register(b0)
{
    float3 colorTint;
    float time;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	//oscillating color using sin waves and time
    return float4(GetColorShift(time) * colorTint, 1);
}


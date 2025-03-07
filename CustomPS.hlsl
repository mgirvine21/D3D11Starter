
cbuffer ExternalData : register(b0)
{
    float3 colorTint;
    float time;
}

// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

//function to make a smooth color change using sin
float3 GetColorShift(float t)
{
    return float3(
        sin(t * 0.5) * 0.5 + 0.5, // Red channel oscillation
        sin(t * 0.7 + 2.0) * 0.5 + 0.5, // Green channel oscillation
        sin(t * 0.9 + 4.0) * 0.5 + 0.5 // Blue channel oscillation
    );
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


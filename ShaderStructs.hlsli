#ifndef __GPP_SHADER_STRUCTS__
#define __GPP_SHADER_STRUCTS__

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition : POSITION; // XYZ position
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent: TANGENT;
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION; // XYZW position (System Value Position)
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 worldPos : POSITION;
};

struct VertexToPixel_Sky
{
    float4 position : SV_POSITION;
    float3 sampleDir : DIRECTION; // For cube map sampling
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

#endif

#include "ShaderStructs.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// --------------------------------------------------------
VertexToPixel_Sky main(VertexShaderInput input)
{
    VertexToPixel_Sky output;

	//remove translation from view matrix by setting values to 0
    matrix viewNoTranslation = view;
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;

	//multiply new veiw and projection matrices
    matrix vp = mul(projection, viewNoTranslation);
    output.position = mul(vp, float4(input.localPosition, 1.0f));

	// set sky vertex to be on far clip plan
    output.position.z = output.position.w;

	//vert position for direction
    output.sampleDir = input.localPosition;

    return output;
}
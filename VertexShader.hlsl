#include "ShaderStructs.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix worldMatrix;
    matrix worldInvTrans;
    matrix viewMatrix;
    matrix projectionMatrix;
	
    matrix lightView;
    matrix lightProjection;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in the future).
    matrix wvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	//passing through other data
    output.uv = input.uv;
    output.normal = normalize(mul((float3x3) worldInvTrans, input.normal));
    output.tangent = normalize(mul((float3x3) worldMatrix, input.tangent));
    output.worldPos = mul(worldMatrix, float4(input.localPosition, 1.0f)).xyz;
	
	//calculate where this vertex is from lights pov
    matrix shadowWVP = mul(lightProjection, mul(lightView, worldMatrix));
    output.shadowMapPos = mul(shadowWVP, float4(input.localPosition, 1.0f));
	
	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}
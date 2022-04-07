struct VertexShaderOutput
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

VertexShaderOutput VS_main(
	float4 position : POSITION,
	float2 uv : TEXCOORD)
{
	VertexShaderOutput output;

	output.position = position;
	output.uv = uv;

	return output;
}

float4 PS_main (float4 position : SV_POSITION,
				float2 uv : TEXCOORD) : SV_TARGET
{
	return float4(uv, 0, 1);
}


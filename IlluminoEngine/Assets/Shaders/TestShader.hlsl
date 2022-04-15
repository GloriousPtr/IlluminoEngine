struct VertexIn
{
	float4 position : POSITION;
	float2 uv : TEXCOORD;
};

struct VertexOut
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer Uniforms : register (b0)
{
	float4 colorR;
}

VertexOut VS_main(VertexIn v)
{
	VertexOut output;

	output.position = v.position;
	output.uv = v.uv;

	return output;
}

float4 PS_main(VertexOut i) : SV_TARGET
{
	return float4(colorR.r, 0, 0, 1);
//	return float4(i.uv, 0, 1);
}

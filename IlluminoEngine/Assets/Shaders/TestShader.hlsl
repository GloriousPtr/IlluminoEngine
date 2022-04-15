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

cbuffer Properties : register (b0)
{
	float4x4 u_Transform;
	float4 u_Color;
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
	return float4(u_Color.rgb, 1);
}

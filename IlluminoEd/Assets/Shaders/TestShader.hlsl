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
	row_major float4x4 u_MVP;
	float4 u_Color;
}

VertexOut VS_main(VertexIn v)
{
	VertexOut output;

	output.position = mul(v.position, u_MVP);
	output.uv = v.uv;

	return output;
}

Texture2D<float4> u_Texture : register(t0);
SamplerState u_TextureSampler : register(s0);

float4 PS_main(VertexOut i) : SV_TARGET
{
	float4 albedo = u_Texture.Sample(u_TextureSampler, i.uv);
	if (albedo.a < 0.05)
		discard;

	return float4(albedo.rgb, 1.0f);
}

//--------------------------------------------------------------------------------------
cbuffer cbNeverChanges : register(b0)
{
	matrix Projection;
}

Texture2D		txAlbedo : register(t0);
SamplerState	smAlbedo : register(s0);

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float4 Pos : POSITION;
	float4 Col : COLOR;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos		: SV_POSITION;
	float4 Col		: COLOR;
	float2 Tex		: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(float4(input.Pos.xyz, 1), Projection);
	output.Tex = input.Tex;
	output.Col = input.Col;
	return output;
}

//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	float4 color = txAlbedo.Sample(smAlbedo, input.Tex);

	clip(color.w - 1/255.0);

	return color * input.Col;
}


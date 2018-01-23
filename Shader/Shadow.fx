cbuffer cbNeverChanges : register(b0)
{
	matrix WorldViewProjection;
	matrix invWorldViewT;
	float4 EyePos;
};

struct VS_INPUT
{
	float4 Pos : POSITION;
};

struct PS_INPUT
{
	float4 Pos	: SV_POSITION;
	float W		: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(float4(input.Pos.xyz, 1), WorldViewProjection);
	output.W = output.Pos.w;
	return output;
}

//--------------------------------------------------------------------------------------
float PS(PS_INPUT input) : SV_Target
{
	return input.W;
}


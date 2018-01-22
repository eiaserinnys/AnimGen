//--------------------------------------------------------------------------------------
cbuffer cbNeverChanges : register(b0)
{
	matrix World;
	matrix ViewProjection;
	float4 EyePos;
};

struct VS_INPUT
{
	float4 Pos : POSITION;
	float4 Nor : NORMAL;
	float4 Col : COLOR;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Nor : TEXCOORD0;
	float4 Col : COLOR;
};

//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(float4(input.Pos.xyz, 1), World);
	output.Pos = mul(output.Pos, ViewProjection);
	output.Nor = input.Nor;
	output.Col = input.Col;
	return output;
}

//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	return input.Col;
}


//--------------------------------------------------------------------------------------
cbuffer cbNeverChanges : register(b0)
{
	matrix WorldViewProjection;
	matrix invWorldViewT;
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
	float4 Pos		: SV_POSITION;
	float4 Pos2		: TEXCOORD0;
	float4 Nor		: TEXCOORD1;
	float4 Col		: COLOR;
};

struct PS_OUTPUT
{
	float4	Col		: SV_Target0;
	float4	Nor		: SV_Target1;
	float4	View	: SV_Target2;
	float4	Depth	: SV_Target3;
};

//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(float4(input.Pos.xyz, 1), WorldViewProjection);
	output.Pos2 = output.Pos;
	output.Nor = input.Nor;
	output.Col = input.Col;
	return output;
}

//--------------------------------------------------------------------------------------
PS_OUTPUT PS(PS_INPUT input) : SV_Target
{
	PS_OUTPUT output = (PS_OUTPUT)0;
	output.Col = input.Col;
	output.Nor = float4((input.Nor.xyz + 1) / 2, 1);
	output.Depth = float4(input.Pos2.w, 0, 0, 1);
	return output;
}


#include "pch.h"
#include "DiagnosticRenderer.h"

#include <DX11Buffer.h>
#include <DX11Shader.h>
#include <DX11InputLayout.h>
#include <DX11StateBlocks.h>
#include <DX11ConstantBufferT.h>
#include <SimpleConstantBuffer.h>

#include <Utility.h>
#include <WindowsUtility.h>

#include "RenderContext.h"
#include "FrameHelper.h"

#include "Vector.h"
#include "ExponentialMap.h"
#include "DXMathTransform.h"
#include "SceneDescriptor.h"

#include "SplineDiagnostic.h"
#include "LineBuffer.h"

using namespace std;
using namespace Core;
using namespace DirectX;

//------------------------------------------------------------------------------
class DiagnosticRenderer : public IDiagnosticRenderer {
public:
	//--------------------------------------------------------------------------
	DiagnosticRenderer(RenderContext* context)
		: context(context)
	{
		ID3D11Device* d3dDev = context->d3d11->g_pd3dDevice;
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		rasterState.reset(IDX11RasterizerState::Create_Default(d3dDev));
		depthState.reset(IDX11DepthStencilState::Create_Default(d3dDev));
		depthStateWire.reset(IDX11DepthStencilState::Create_Always(d3dDev));
		blendState.reset(IDX11BlendState::Create_Default(d3dDev));

		constants.reset(new SimpleConstant(d3dDev, devCtx));

		lineBuffer.reset(new LineBuffer(d3dDev));
	}

	//--------------------------------------------------------------------------
	void Render(const SceneDescriptor& sceneDesc)
	{
		ID3D11DeviceContext* devCtx = context->d3d11->immDevCtx;

		lineBuffer->End(devCtx);

		context->d3d11->immDevCtx->ClearState();
		context->rts->Restore("deferred");

		constants->Update(
			devCtx,
			sceneDesc.worldViewProj,
			sceneDesc.invWorldViewT,
			sceneDesc.eye);

		// 상수들
		rasterState->Apply(devCtx);
		depthState->Apply(devCtx);
		blendState->Apply(devCtx);

		// 쉐이더 설정
		if (context->sd->Set("Diagnostic"))
		{ 
			lineBuffer->Draw(devCtx);
		}		
	}

	//--------------------------------------------------------------------------
	LineBuffer* Buffer() { return lineBuffer.get(); }

	//--------------------------------------------------------------------------
	RenderContext* context;

	unique_ptr<SimpleConstant> constants;

	unique_ptr<IDX11RasterizerState> rasterState;
	unique_ptr<IDX11DepthStencilState> depthState;
	unique_ptr<IDX11BlendState> blendState;

	unique_ptr<IDX11RasterizerState> rasterStateWire;
	unique_ptr<IDX11DepthStencilState> depthStateWire;

	unique_ptr<LineBuffer> lineBuffer;
	unique_ptr<SplineDiagnostic> test;
};

//------------------------------------------------------------------------------
IDiagnosticRenderer::~IDiagnosticRenderer()
{}

//------------------------------------------------------------------------------
IDiagnosticRenderer* IDiagnosticRenderer::Create(RenderContext* context)
{ return new DiagnosticRenderer(context); }

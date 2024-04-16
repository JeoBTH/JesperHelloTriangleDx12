#include "Renderer.h"

void Renderer::createDevice(Window& window)
{

}

void Renderer::createRenderTarget()
{

}

void Renderer::CreateDXGIFactory2()
{

	if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&m_dxgiFactory))))
	{

	}


}

void Renderer::CreateDirect3DDevice()
{
	//dxgi1_6 is only needed for the initialization process using the adapter.
	IDXGIAdapter1* adapter = nullptr;
	D3D_FEATURE_LEVEL featureLevelToUse = D3D_FEATURE_LEVEL_12_1;

	for (uint32_t adapterIndex = 0;; ++adapterIndex)
	{
		adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == m_dxgiFactory->EnumAdapters1(adapterIndex, &adapter))
		{
			break; //No more adapters to enumerate.
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, featureLevelToUse, __uuidof(ID3D12Device5), nullptr)))
		{
			break;
		}

		SafeRelease(adapter);
	}

}

Renderer::Renderer(Window& window)
{
}

Renderer::~Renderer()
{
}

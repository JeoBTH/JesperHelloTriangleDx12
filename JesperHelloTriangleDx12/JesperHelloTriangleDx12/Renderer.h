#pragma once
#include "Window.h"
#include <d3d12.h>
#include <dxgi1_6.h> // swapchain
#include <vector>


class Renderer
{
private:

	// Device
	IDXGISwapChain* m_SwapChain = nullptr;
	ID3D12Device* m_device = nullptr;
	// ID3D11DeviceContext* m_deviceContext = nullptr; deviceContext doesn't exist in dx12?


	// Render Target

	std::vector<ID3D12Resource*> m_renderTargetView; // was ID3D11RenderTargetView
	D3D12_CPU_DESCRIPTOR_HANDLE m_heapHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_backBufferDesc; // was D3D11_TEXTURE2D_DESC
	ID3D12DescriptorHeap* m_descriptorHeap;

	void createDevice(Window& window);
	void createRenderTarget();
public:
	Renderer(Window& window);
	~Renderer();



};





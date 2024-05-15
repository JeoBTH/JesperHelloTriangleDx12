#pragma once
#include "Window.h"
#include <d3d12.h>
#include <dxgi1_6.h> // swapchain
#include <d3dcompiler.h>
#include <vector>

#include <d3dcompiler.h>
#include <cstdint>

#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "DXGI.lib")
#pragma comment (lib, "d3dcompiler.lib")

class Renderer
{
private:

	// Device
	IDXGIFactory5* m_dxgiFactory = nullptr;
	ID3D12Device4* m_dx12Device = nullptr;
	ID3D12CommandQueue* m_dx12CommandQueue = nullptr;
	ID3D12CommandAllocator* m_dx12CommandAllocator = nullptr;
	ID3D12GraphicsCommandList3* m_dx12CommandList = nullptr;
	IDXGISwapChain4* m_dxgiSwapChain = nullptr;
	const uint32_t NUM_SWAP_BUFFERS = 2; //Number of buffers
	ID3D12Fence1* m_dx12Fence = nullptr;
	ID3D12DescriptorHeap* m_dx12RenderTargetsHeap = nullptr;
	uint32_t m_renderTargetDescriptorSize = 0;
	ID3D12Resource1* m_dx12RenderTargets[2] = {};

	// Viewport
	D3D12_VIEWPORT m_dx12Viewport = {};
	const uint32_t SCREEN_WIDTH = 640; //Width of application.
	const uint32_t SCREEN_HEIGHT = 480;	//Height of application.
	D3D12_RECT m_dx12ScissorRect = {};

	typedef int myint;
	uint64_t m_fenceValue = 0;

	ID3D12DescriptorHeap* m_gDescriptorHeap[2] = {};
	ID3D12Resource1* m_gConstantBufferResource[2] = {};

	ID3D12RootSignature* m_dx12RootSignature = nullptr;
	ID3D12PipelineState* m_dx12PipeLineState = nullptr;

	uint32_t m_backBufferIndex = 0;

	// WaitForGPU
	const uint32_t BUSY_WAIT = 0;
	HANDLE m_eventHandle = nullptr;

	// Render Target
	void CreateDXGIFactory2();								//2. Create Device
	void CreateDirect3DDevice();							//3. Create Device
	void CreateCommandInterfaces();							//4. Create CommandQueue and SwapChain
	void CreateSwapChain(Window& window);					//5. Create CommandQueue and SwapChain
	void CreateFence();										//6. Create Fence
	void CreateRenderTargets();								//7. Create render targets for backbuffer
	void CreateViewportAndScissorRect();					//8. Create viewport and rect
	void CreateConstantBufferResources();					//11. Create constant buffer data
	void CreateRootSignature();								//9. Create root signature

	void SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource,
		D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter);

	void WaitForGPU();

	template<class Interface>
	inline void SafeRelease(Interface*& ppInterfaceToRelease)
	{
		if (nullptr != ppInterfaceToRelease)
		{
			ppInterfaceToRelease->Release();
			ppInterfaceToRelease = nullptr;
		}
	}

public:
	Renderer(Window& window);
	~Renderer();

	void BeginFrame(uint32_t frameResourceIndex);
	void EndFrame();

	ID3D12Device4* getDevice();
	ID3D12GraphicsCommandList3* getCommandList();
	ID3D12RootSignature* getRootSignature();
	ID3D12PipelineState** getPipelineState();

};





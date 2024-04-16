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



	// Render Target

	void createDevice(Window& window);
	void createRenderTarget();

	void CreateDXGIFactory2();								//2. Create Device
	void CreateDirect3DDevice();							//3. Create Device
	//void CreateCommandInterfaces();							//4. Create CommandQueue and SwapChain
	//void CreateSwapChain(Window& window);					//5. Create CommandQueue and SwapChain
	//void CreateFence();										//6. Create Fence
	//void CreateRenderTargets();								//7. Create render targets for backbuffer
	//void CreateViewportAndScissorRect();					//8. Create viewport and rect
	//void CreateRootSignature();								//9. Create root signature
	//void CreateShadersAndPiplelineState();					//10. Set up the pipeline state
	//void CreateConstantBufferResources();					//11. Create constant buffer data
	//void CreateTriangleData();								//12. Create vertexdata

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



};





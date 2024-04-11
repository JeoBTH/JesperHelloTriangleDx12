#include "Renderer.h"

void Renderer::createDevice(Window& window)
{

}

void Renderer::createRenderTarget()
{
	ID3D12Resource* backBuffer = nullptr; // was ID3D11Texture2D
	

	this->m_heapHandle = this->m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

	this->m_SwapChain->GetBuffer(0, __uuidof(ID3D12Resource), (void**)&backBuffer); // how you get the buffer from the swap chain. index of buffer, what type, to where.
	this->m_device->CreateRenderTargetView(backBuffer, nullptr, m_heapHandle);



	m_renderTargetView.push_back(backBuffer);
		
	//was backBuffer->GetDesc(&m_backBufferDesc);


	backBuffer->Release(); // everytime you get a buffer, it will increase its index count. So you have to release to decrement
}

Renderer::Renderer(Window& window)
{
}

Renderer::~Renderer()
{
}

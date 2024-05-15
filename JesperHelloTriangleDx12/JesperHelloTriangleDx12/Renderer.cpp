#include "Renderer.h"

struct ConstantBuffer
{
	float luminance;
	float _PADDING_[3];
};

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

	if (adapter)
	{
		//Create the actual device.
		if (SUCCEEDED(D3D12CreateDevice(adapter, featureLevelToUse, IID_PPV_ARGS(&m_dx12Device))))
		{

		}

		SafeRelease(adapter);
	}
	else
	{
		//Create warp device if no adapter was found.
		m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter));
		D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_dx12Device));
	}


}

void Renderer::CreateCommandInterfaces()
{
	{
		//Describe and create the command queue.
		D3D12_COMMAND_QUEUE_DESC cqd = {};
		m_dx12Device->CreateCommandQueue(&cqd, IID_PPV_ARGS(&m_dx12CommandQueue));

		//Create command allocator. The command allocator object corresponds
		//to the underlying allocations in which GPU commands are stored.
		m_dx12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_dx12CommandAllocator));

		//Create command list.
		m_dx12Device->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_dx12CommandAllocator,
			nullptr,
			IID_PPV_ARGS(&m_dx12CommandList));

		//Command lists are created in the recording state. Since there is nothing to
		//record right now and the main loop expects it to be closed, we close it.
		m_dx12CommandList->Close();
	}
}

void Renderer::CreateSwapChain(Window& window)
{
	//Create swap chain.
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = 0;
	scDesc.Height = 0;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = NUM_SWAP_BUFFERS;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.Flags = 0;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	IDXGISwapChain1* swapChain1 = nullptr;
	if (SUCCEEDED(m_dxgiFactory->CreateSwapChainForHwnd(
		m_dx12CommandQueue,
		window.getHandle(),
		&scDesc,
		nullptr,
		nullptr,
		&swapChain1)))
	{
		if (SUCCEEDED(swapChain1->QueryInterface(IID_PPV_ARGS(&m_dxgiSwapChain))))
		{
			m_dxgiSwapChain->Release();
		}
	}

}

void Renderer::CreateFence()
{
	m_fenceValue = 0;
	m_dx12Device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_dx12Fence));
}

void Renderer::CreateRenderTargets()
{
	//Create descriptor heap for render target views.
	D3D12_DESCRIPTOR_HEAP_DESC dhd = {};
	dhd.NumDescriptors = NUM_SWAP_BUFFERS;
	dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	HRESULT hr = m_dx12Device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(&m_dx12RenderTargetsHeap));

	//Create resources for the render targets.
	m_renderTargetDescriptorSize = m_dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_dx12RenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();

	//One RTV for each frame.
	for (uint32_t n = 0; n < NUM_SWAP_BUFFERS; n++)
	{
		hr = m_dxgiSwapChain->GetBuffer(n, IID_PPV_ARGS(&m_dx12RenderTargets[n]));
		m_dx12Device->CreateRenderTargetView(m_dx12RenderTargets[n], nullptr, cdh);
		cdh.ptr += m_renderTargetDescriptorSize;
	}

}

void Renderer::CreateViewportAndScissorRect()
{
	m_dx12Viewport.TopLeftX = 0.0f;
	m_dx12Viewport.TopLeftY = 0.0f;
	m_dx12Viewport.Width = (float)SCREEN_WIDTH;
	m_dx12Viewport.Height = (float)SCREEN_HEIGHT;
	m_dx12Viewport.MinDepth = 0.0f;
	m_dx12Viewport.MaxDepth = 1.0f;

	m_dx12ScissorRect.left = (long)0;
	m_dx12ScissorRect.right = (long)SCREEN_WIDTH;
	m_dx12ScissorRect.top = (long)0;
	m_dx12ScissorRect.bottom = (long)SCREEN_HEIGHT;
}

void Renderer::CreateConstantBufferResources()
{
	for (uint32_t i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDescriptorDesc = {};
		heapDescriptorDesc.NumDescriptors = 1;
		heapDescriptorDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDescriptorDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		m_dx12Device->CreateDescriptorHeap(&heapDescriptorDesc, IID_PPV_ARGS(&m_gDescriptorHeap[i]));
	}

	uint32_t cbSizeAligned = (sizeof(ConstantBuffer) + 255) & ~255;	// 256-byte aligned CB.

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.CreationNodeMask = 1; //used when multi-gpu
	heapProperties.VisibleNodeMask = 1; //used when multi-gpu
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = cbSizeAligned;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//Create a resource heap, descriptor heap, and pointer to cbv for each frame
	for (uint32_t i = 0; i < NUM_SWAP_BUFFERS; i++)
	{
		m_dx12Device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_gConstantBufferResource[i])
		);

		m_gConstantBufferResource[i]->SetName(L"cb heap");

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = m_gConstantBufferResource[i]->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = cbSizeAligned;
		m_dx12Device->CreateConstantBufferView(&cbvDesc, m_gDescriptorHeap[i]->GetCPUDescriptorHandleForHeapStart());
	}
}

void Renderer::CreateRootSignature()
{
	//define descriptor range(s)
	D3D12_DESCRIPTOR_RANGE  dtRanges[1];
	dtRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	dtRanges[0].NumDescriptors = 1; //only one CB in this example
	dtRanges[0].BaseShaderRegister = 0; //register b0
	dtRanges[0].RegisterSpace = 0; //register(b0,space0);
	dtRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE dt;
	dt.NumDescriptorRanges = ARRAYSIZE(dtRanges);
	dt.pDescriptorRanges = dtRanges;

	//create root parameter
	D3D12_ROOT_PARAMETER  rootParam[1]{};
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable = dt;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_DESC rsDesc;
	rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsDesc.NumParameters = ARRAYSIZE(rootParam);
	rsDesc.pParameters = rootParam;
	rsDesc.NumStaticSamplers = 0;
	rsDesc.pStaticSamplers = nullptr;

	ID3DBlob* sBlob;
	D3D12SerializeRootSignature(
		&rsDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&sBlob,
		nullptr);

	m_dx12Device->CreateRootSignature(
		0,
		sBlob->GetBufferPointer(),
		sBlob->GetBufferSize(),
		IID_PPV_ARGS(&m_dx12RootSignature));
}

void Renderer::SetResourceTransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter)
{
	D3D12_RESOURCE_BARRIER barrierDesc = {};

	barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrierDesc.Transition.pResource = resource;
	barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrierDesc.Transition.StateBefore = StateBefore;
	barrierDesc.Transition.StateAfter = StateAfter;

	commandList->ResourceBarrier(1, &barrierDesc);
}

void Renderer::WaitForGPU()
{
	//WAITING FOR EACH FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	//This is code implemented as such for simplicity. The cpu thread could for example be used
	//for other tasks to prepare the next frame while the current one is being rendered.

	//Signal fence using pre-incremented fence value.
	m_dx12CommandQueue->Signal(m_dx12Fence, ++m_fenceValue);

	if(BUSY_WAIT)
		// Could give some time back in each iteration...
		while (m_dx12Fence->GetCompletedValue() < m_fenceValue);
	else
		// This path using OS primitives (SetEventOnCompletion/WaitForSingleObject)
		// will give CPU time back to the OS.
		// Only do busy waits when motivated and really needed.
		if (m_dx12Fence->GetCompletedValue() < m_fenceValue)
		{
			//Create an event handle to use for GPU synchronization.
			if (m_eventHandle == NULL) {
				m_eventHandle = CreateEvent(0, false, false, 0);
			}

			m_dx12Fence->SetEventOnCompletion(m_fenceValue, m_eventHandle);
			if (m_eventHandle!=0)
			{
				WaitForSingleObject(m_eventHandle, INFINITE);
			}
	}
}

void Renderer::BeginFrame(uint32_t frameResourceIndex)
{
	//Command list allocators can only be reset when the associated command lists have
	//finished execution on the GPU; fences are used to ensure this (See WaitForGpu method)
	m_dx12CommandAllocator->Reset();
	m_dx12CommandList->Reset(m_dx12CommandAllocator, m_dx12PipeLineState);

	//Set constant buffer descriptor heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { m_gDescriptorHeap[frameResourceIndex] };
	m_dx12CommandList->SetDescriptorHeaps(ARRAYSIZE(descriptorHeaps), descriptorHeaps);

	//Set root signature
	m_dx12CommandList->SetGraphicsRootSignature(m_dx12RootSignature);

	//Set root descriptor table to index 0 in previously set root signature
	m_dx12CommandList->SetGraphicsRootDescriptorTable(0,
		m_gDescriptorHeap[frameResourceIndex]->GetGPUDescriptorHandleForHeapStart());

	//Set necessary states.
	m_dx12CommandList->RSSetViewports(1, &m_dx12Viewport);
	m_dx12CommandList->RSSetScissorRects(1, &m_dx12ScissorRect);

	//Indicate that the back buffer will be used as render target.
	//NOTE: Backbuffer index is not the same as frameResourceIndex!
	m_backBufferIndex = m_dxgiSwapChain->GetCurrentBackBufferIndex();
	SetResourceTransitionBarrier(m_dx12CommandList,
		m_dx12RenderTargets[this->m_backBufferIndex],
		D3D12_RESOURCE_STATE_PRESENT,		//state before
		D3D12_RESOURCE_STATE_RENDER_TARGET	//state after
	);

	//Record commands.
	//Get the handle for the current render target used as back buffer.
	D3D12_CPU_DESCRIPTOR_HANDLE cdh = m_dx12RenderTargetsHeap->GetCPUDescriptorHandleForHeapStart();
	cdh.ptr += m_renderTargetDescriptorSize * m_backBufferIndex;

	m_dx12CommandList->OMSetRenderTargets(1, &cdh, true, nullptr);

	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	m_dx12CommandList->ClearRenderTargetView(cdh, clearColor, 0, nullptr);



}

void Renderer::EndFrame()
{

	//Indicate that the back buffer will now be used to present.
	SetResourceTransitionBarrier(m_dx12CommandList,
		m_dx12RenderTargets[m_backBufferIndex],
		D3D12_RESOURCE_STATE_RENDER_TARGET,	//state before
		D3D12_RESOURCE_STATE_PRESENT		//state after
	);

	//Close the list to prepare it for execution.
	m_dx12CommandList->Close();

	//Execute the command list.
	ID3D12CommandList* listsToExecute[] = { m_dx12CommandList };
	m_dx12CommandQueue->ExecuteCommandLists(ARRAYSIZE(listsToExecute), listsToExecute);

	//Present the frame.
	DXGI_PRESENT_PARAMETERS pp = {};
	m_dxgiSwapChain->Present1(0, 0, &pp);

	WaitForGPU(); //Wait for GPU to finish.
	//NOT BEST PRACTICE, only used as such for simplicity.
}

Renderer::Renderer(Window& window)
{
	CreateDXGIFactory2();								//2. Create Device
	CreateDirect3DDevice();								//3. Create Device
	CreateCommandInterfaces();							//4. Create CommandQueue and SwapChain
	CreateSwapChain(window);							//5. Create CommandQueue and SwapChain
	CreateFence();										//6. Create Fence
	CreateRenderTargets();								//7. Create render targets for backbuffer
	CreateViewportAndScissorRect();						//8. Create viewport and rect
	CreateRootSignature();								//9. Create root signature
	CreateConstantBufferResources();					//11. Create constant buffer data

	WaitForGPU();
}

Renderer::~Renderer()
{
	WaitForGPU();	//Wait for GPU execution to be done and then release all interfaces.

	SafeRelease(m_dx12Device);
	SafeRelease(m_dx12CommandQueue);
	SafeRelease(m_dx12CommandAllocator);
	SafeRelease(m_dx12CommandList);
	SafeRelease(m_dxgiSwapChain);
	SafeRelease(m_dx12Fence);
	SafeRelease(m_dx12RenderTargetsHeap);

	for (int i = 0; i < 2; i++)
	{
		SafeRelease(m_gDescriptorHeap[i]);
		SafeRelease(m_gConstantBufferResource[i]);
		SafeRelease(m_dx12RenderTargets[i]);
	}

	SafeRelease(m_dx12RootSignature);
	SafeRelease(m_dx12PipeLineState);
	SafeRelease(m_dxgiFactory);
}

ID3D12Device4* Renderer::getDevice()
{
	return this->m_dx12Device;
}

ID3D12GraphicsCommandList3* Renderer::getCommandList()
{
	return this->m_dx12CommandList;
}

ID3D12RootSignature* Renderer::getRootSignature()
{
	return this->m_dx12RootSignature;
}

ID3D12PipelineState** Renderer::getPipelineState()
{
	return &this->m_dx12PipeLineState;
}

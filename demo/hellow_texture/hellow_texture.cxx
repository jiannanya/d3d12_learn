
#include "hellow_texture.hxx"
#include "texture_helper.hxx"

#include <d3dx12_core.h>
#include <d3dcompiler.h>
#include <d3dx12_barriers.h>
#include <d3dx12_root_signature.h>
#include <d3dx12_resource_helpers.h>
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <spdlog/spdlog.h>

CD3D12Texture::CD3D12Texture(const std::string& title)
    : Application(title)
{
	m_rtvDescriptorSize = 0;
	m_frameIndex = 0;
	m_hWnd = NULL;
	WCHAR assetsPath[512] = {0};
	GetCurrentPath(assetsPath, _countof(assetsPath));
	m_CurrentPath  = assetsPath;
}

CD3D12Texture::~CD3D12Texture(void)
{

}

void CD3D12Texture::OnInit(HWND h)
{
	m_nSRVDescriptorSize = 0;
	m_hWnd = h;
	UINT flags = OpenDebug();
	CreateDevice(flags, m_factory, m_device);  
	CreateCommandQueue(m_commandQueue);
	
	RECT rect = { 0 };
	GetClientRect(m_hWnd, &rect);
	CreateSwapChain(m_commandQueue.Get(), rect.right, rect.bottom, m_swapChain); 
	
	//create rtv
	CreateRenderTargetViewHeap(m_rtvHeap); 
	CreateRenderTargetView(m_renderTargets); 

	CreateRootSignature(m_rootSignature); 
	CreateGPUPipelineState(m_pipelineState,m_commandAllocator, m_commandList);  
	InitializeVertexBuffer(m_vertexBuffer,m_vertexBufferView);

	SetCreateTexture2D();

#if __CBV__	
	CreateConstantBuffer(m_srvHeap, m_constantBuffer);
    spdlog::info("Constant buffer created.");
#endif	
	
	CreateCpuAndGpuSynchronization();  

	WaitForPreviousFrame();        
}
void CD3D12Texture::OnRender(void)
{

#if __CBV__
	UpdateConstantBufferData();
#endif

	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();


}
void CD3D12Texture::OnDestroy(void)
{
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
#if __CBV__
	m_constantBuffer->Unmap(0, nullptr);
#endif

}

void CD3D12Texture::onInit(GLFWwindow* window)
{
    OnInit(glfwGetWin32Window(window));
    
}

bool CD3D12Texture::onLoad()
{
    return true;
}


void CD3D12Texture::onUpdate()
{
}

void CD3D12Texture::onRender()
{
	OnRender();
}

void CD3D12Texture::onDestroy()
{
	OnDestroy();
}

UINT CD3D12Texture::OpenDebug(void)
{
	UINT dxgiFactoryFlags = 0;

	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}

	return dxgiFactoryFlags;
}

void CD3D12Texture::CreateDevice(UINT dxgiFactoryFlags, ComPtr<IDXGIFactory6>& factory, ComPtr<ID3D12Device>& device)
{
	ComPtr<IDXGIAdapter1> adapter;
	HRESULT hr = S_OK;
	(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
	
	for (int i = 0; SUCCEEDED(factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))); i++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
		{
			SetWindowTextW(m_hWnd, desc.Description);
			ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));
			break;
		}
	}

	ThrowIfFailed(factory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));
}

void CD3D12Texture::CreateCommandQueue(ComPtr<ID3D12CommandQueue>& commandQueue)
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
}

void CD3D12Texture::CreateSwapChain(ID3D12CommandQueue* pQueue, int w, int h, ComPtr<IDXGISwapChain3>& swapChain3)
{

	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(w), static_cast<LONG>(h));

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = m_nFrameCount;
	swapChainDesc.Width = w;
	swapChainDesc.Height = h;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	ComPtr<IDXGISwapChain1> swapChain;
	
	ThrowIfFailed(m_factory->CreateSwapChainForHwnd(
		pQueue,       
		m_hWnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));
	
	ThrowIfFailed(swapChain.As(&swapChain3));
	m_frameIndex = swapChain3->GetCurrentBackBufferIndex();
}

void CD3D12Texture::CreateRenderTargetViewHeap(ComPtr<ID3D12DescriptorHeap>& heap)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = m_nFrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&heap)));


}

void CD3D12Texture::CreateRenderTargetView(ComPtr<ID3D12Resource> rtv[])
{
	
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create a RTV for each frame.
	for (int i = 0; i < m_nFrameCount; i++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&rtv[i])));
		m_device->CreateRenderTargetView(rtv[i].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}
	 
}

void CD3D12Texture::CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}

	CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
	CD3DX12_ROOT_PARAMETER1 rootParameters[2];

	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	///  __CBV__
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
	///  __CBV__

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;

	rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

}

void CD3D12Texture::ShadersCompileFromFile(LPCWSTR pFileName, LPCSTR pEntrypoint, LPCSTR pTarget, ComPtr<ID3DBlob>& shader)
{
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	ThrowIfFailed(D3DCompileFromFile(pFileName, nullptr, nullptr, pEntrypoint, pTarget, compileFlags, 0, &shader, nullptr));
}

void CD3D12Texture::CreateGPUPipelineState(ComPtr<ID3D12PipelineState>& pipelineState, ComPtr<ID3D12CommandAllocator>& commandAllocator, ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

	ShadersCompileFromFile(GetShaderFilePath(L"Texture.hlsl").c_str(), "VSMain", "vs_5_0", vertexShader);
	ShadersCompileFromFile(GetShaderFilePath(L"Texture.hlsl").c_str(), "PSMain", "ps_5_0", pixelShader);
	
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = m_rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&commandList)));
	///ThrowIfFailed(commandList->Close());

}

void CD3D12Texture::InitializeVertexBuffer(ComPtr<ID3D12Resource>& vertexBuffer,D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
{
	Vertex triangleVertices[] =
	{
		{ { -0.8, -0.8, 0.0f }, { 0.0f, 0.5f} },
		{ { -0.8, 0.8, 0.0f }, { 0.0f, 0.0f} },
		{ { 0.8, -0.8, 0.0f }, { 0.5f, 0.5f } },
		{ { 0.8, 0.8, 0.0f }, { 0.5f, 0.0f } },
		
	};

	const UINT vertexBufferSize = sizeof(triangleVertices);
	auto pri = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto res = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	ThrowIfFailed(m_device->CreateCommittedResource(
		&pri,
		D3D12_HEAP_FLAG_NONE,
		&res,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)));

	UINT8* pVertexDataBegin = NULL;
	CD3DX12_RANGE readRange(0, 0);        
	ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
	memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
	vertexBuffer->Unmap(0, nullptr);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vertexBufferSize;

}


void CD3D12Texture::SetCreateTexture2D()
{
	UINT w = 256;
	UINT h = 256;
	UINT bpp = 4;
	DXGI_FORMAT dxFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	UINT nPicRowPitch = 0;
	BYTE* pImageData = NULL;

	CWICIamge textureImg;
	dxFormat = textureImg.GetImageInfo(GetShaderFilePath(L"Texture.png").c_str(), w, h, bpp, nPicRowPitch, &pImageData);

	CreateDefaultHeapTexture(m_texture, dxFormat, w, h);
	CreateUploadHeapTexture(m_textureUploadHeap, w, h);
	CreateShaderResource(dxFormat);

	CopyImageDataCommitted(w,h,bpp, pImageData);
	delete[]pImageData;
}

void CD3D12Texture::CopyImageDataCommitted(int w,int h,int bpp, BYTE* pImageData)
{
	D3D12_SUBRESOURCE_DATA textureData = {};
	textureData.pData = pImageData;
	textureData.RowPitch = w * bpp / 8;
	textureData.SlicePitch = textureData.RowPitch * h;

	UpdateSubresources(m_commandList.Get(), m_texture.Get(), m_textureUploadHeap.Get(), 0, 0, 1, &textureData);

	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_commandList->ResourceBarrier(1, &barrier);
	
	ThrowIfFailed(m_commandList->Close());

	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}


void CD3D12Texture::CreateDefaultHeapTexture(ComPtr<ID3D12Resource>& texture, DXGI_FORMAT dxFormat,UINT w, UINT h)
{
	D3D12_RESOURCE_DESC textureDesc = {};
	textureDesc.MipLevels = 1;
	textureDesc.Format = dxFormat;
	textureDesc.Width = w;
	textureDesc.Height = h;
	textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	textureDesc.DepthOrArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	auto pri = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	ThrowIfFailed(m_device->CreateCommittedResource(
		&pri,
		D3D12_HEAP_FLAG_NONE,
		&textureDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&texture)));

}
void CD3D12Texture::CreateUploadHeapTexture(ComPtr<ID3D12Resource>& texture, UINT w, UINT h)
{
	const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_texture.Get(), 0, 1);

	auto pri = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto res = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);

	// Create the GPU upload buffer.
	ThrowIfFailed(m_device->CreateCommittedResource(
		&pri,
		D3D12_HEAP_FLAG_NONE,
		&res,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&texture)));
}

void CD3D12Texture::CreateShaderResource(DXGI_FORMAT dxFormat)
{
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 2; // srv +cbv
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = dxFormat;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	m_device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());
	m_nSRVDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void CD3D12Texture::CreateCpuAndGpuSynchronization()
{
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue = 1;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	
}

void CD3D12Texture::WaitForPreviousFrame(void)
{
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void CD3D12Texture::PopulateCommandList(void)
{
	
	ThrowIfFailed(m_commandAllocator->Reset());


	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	ID3D12DescriptorHeap* ppHeaps[] = { m_srvHeap.Get()};
	D3D12_GPU_DESCRIPTOR_HANDLE hsrvHeap = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
	m_commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
	m_commandList->SetGraphicsRootDescriptorTable(0, hsrvHeap);

#if __CBV__	
	 /// SetDescriptorHeaps
	///一次只能设置每种类型的一个描述符堆，这意味着一次最多可以设置 2 个堆， (一个采样器，一次可以设置一个 CBV/SRV/UAV) 。
	hsrvHeap.ptr += m_nSRVDescriptorSize;
	m_commandList->SetGraphicsRootDescriptorTable(1, hsrvHeap);
#endif

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1, &barrier1);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	// Record commands.
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(4, 1, 0, 0);

	auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	
	// Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(1, &barrier2);

	ThrowIfFailed(m_commandList->Close());
}

#if __CBV__

void CD3D12Texture::CreateConstantBuffer(ComPtr<ID3D12DescriptorHeap>& srvHeap, ComPtr<ID3D12Resource>& cbvResource)
{
	/* 
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap)));
	*/
	const UINT constantBufferSize = sizeof(SceneConstantBuffer);    // CB size is required to be 256-byte aligned.

	auto pri = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto res = CD3DX12_RESOURCE_DESC::Buffer(constantBufferSize);

	ThrowIfFailed(m_device->CreateCommittedResource(
		&pri,
		D3D12_HEAP_FLAG_NONE,
		&res,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&cbvResource)));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = cbvResource->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = constantBufferSize;
	D3D12_CPU_DESCRIPTOR_HANDLE startCbv = srvHeap->GetCPUDescriptorHandleForHeapStart();
	startCbv.ptr += m_nSRVDescriptorSize;
	m_device->CreateConstantBufferView(&cbvDesc, startCbv);

	CD3DX12_RANGE readRange(0, 0);        
	ThrowIfFailed(cbvResource->Map(0, &readRange, reinterpret_cast<void**>(&m_pCbvDataBegin)));
	memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
}

void CD3D12Texture::UpdateConstantBufferData()
{

	const float translationSpeed = 0.005f;
	const float offsetBounds = 1.25f;

	m_constantBufferData.offset.x += translationSpeed;
	if (m_constantBufferData.offset.x > offsetBounds)
	{
		m_constantBufferData.offset.x = -offsetBounds;
	}
	memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));
}
#endif
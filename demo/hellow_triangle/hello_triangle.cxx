

#include "hello_triangle.hxx"

#include <d3dx12_core.h>
#include <d3dcompiler.h>
#include <d3dx12_barriers.h>
#include <d3dx12_root_signature.h>
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>




CD3D12Triangle::CD3D12Triangle(const std::string& title)
    : Application(title)
{
	m_rtvDescriptorSize = 0;
	m_frameIndex = 0;
	m_hWnd = NULL;
	WCHAR assetsPath[512];
	GetCurrentPath(assetsPath, _countof(assetsPath));
	m_CurrentPath  = assetsPath;
}

CD3D12Triangle::~CD3D12Triangle(void)
{

}

void CD3D12Triangle::OnInit(HWND h)
{
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
	
	CreateCpuAndGpuSynchronization();  
	WaitForPreviousFrame();        
}
void CD3D12Triangle::OnRender(void)
{

	PopulateCommandList();

	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	
	ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();
}
void CD3D12Triangle::OnDestroy(void)
{
	WaitForPreviousFrame();

	CloseHandle(m_fenceEvent);
}

void CD3D12Triangle::onInit(GLFWwindow* window)
{
    OnInit(glfwGetWin32Window(window));
    
}

bool CD3D12Triangle::onLoad()
{
    return true;
}


void CD3D12Triangle::onUpdate()
{
}

void CD3D12Triangle::onRender()
{
	OnRender();
}

void CD3D12Triangle::onDestroy()
{
	OnDestroy();
}

UINT CD3D12Triangle::OpenDebug(void)
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

void CD3D12Triangle::CreateDevice(UINT dxgiFactoryFlags, ComPtr<IDXGIFactory6>& factory, ComPtr<ID3D12Device>& device)
{
	ComPtr<IDXGIAdapter1> adapter;
	HRESULT hr = S_OK;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));
	
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

	///ThrowIfFailed(factory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER));
}

void CD3D12Triangle::CreateCommandQueue(ComPtr<ID3D12CommandQueue>& commandQueue)
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
}

void CD3D12Triangle::CreateSwapChain(ID3D12CommandQueue* pQueue, int w, int h, ComPtr<IDXGISwapChain3>& swapChain3)
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

void CD3D12Triangle::CreateRenderTargetViewHeap(ComPtr<ID3D12DescriptorHeap>& heap)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = m_nFrameCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&heap)));
}

void CD3D12Triangle::CreateRenderTargetView(ComPtr<ID3D12Resource> rtv[])
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

void CD3D12Triangle::CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature)
{
	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

}

void CD3D12Triangle::ShadersCompileFromFile(LPCWSTR pFileName, LPCSTR pEntrypoint, LPCSTR pTarget, ComPtr<ID3DBlob>& shader)
{
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	ThrowIfFailed(D3DCompileFromFile(pFileName, nullptr, nullptr, pEntrypoint, pTarget, compileFlags, 0, &shader, nullptr));
}

void CD3D12Triangle::CreateGPUPipelineState(ComPtr<ID3D12PipelineState>& pipelineState, ComPtr<ID3D12CommandAllocator>& commandAllocator, ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	ComPtr<ID3DBlob> vertexShader;
	ComPtr<ID3DBlob> pixelShader;

	ShadersCompileFromFile(GetShaderFilePath(L"Shader.hlsl").c_str(), "VSMain", "vs_5_0", vertexShader);
	ShadersCompileFromFile(GetShaderFilePath(L"Shader.hlsl").c_str(), "PSMain", "ps_5_0", pixelShader);
	
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
	ThrowIfFailed(commandList->Close());

}

void CD3D12Triangle::InitializeVertexBuffer(ComPtr<ID3D12Resource>& vertexBuffer,D3D12_VERTEX_BUFFER_VIEW& vertexBufferView)
{
	Vertex triangleVertices[] =
	{
		{ { -0.5, -0.5, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { -0.5, 0.5, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { 0.5, -0.5, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		///{ { 0.5, 0.5, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },

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

void CD3D12Triangle::CreateCpuAndGpuSynchronization()
{
	ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue = 1;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}

void CD3D12Triangle::WaitForPreviousFrame(void)
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

void CD3D12Triangle::PopulateCommandList(void)
{
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(),D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_commandList->ResourceBarrier(1, &barrier1);

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);						 
	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_commandList->DrawInstanced(3, 1, 0, 0);

	auto barrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_commandList->ResourceBarrier(1, &barrier2);

	ThrowIfFailed(m_commandList->Close());
}
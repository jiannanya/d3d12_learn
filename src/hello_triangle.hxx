#pragma once
#ifndef HELLO_TRIANGLE_HXX
#define HELLO_TRIANGLE_HXX

#include "app.hxx"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <d3dx12_core.h>
#include <Windows.h>

#include <string>

using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;
using Microsoft::WRL::ComPtr;

class HrException
{
public:
	HrException(HRESULT hr)
	{
		hr = hr;
		m_error = GetLastError();
	}
	HRESULT m_hr;
	DWORD m_error;
};

class CD3D12Triangle final: public Application
{
public:
	CD3D12Triangle(const std::string& title);
	~CD3D12Triangle();

	 void OnInit(HWND h);
	 void OnRender(void);
	 void OnDestroy(void);

	 struct Vertex
	 {
		 XMFLOAT3 position;
		 XMFLOAT4 color;
	 };

protected:
	void onInit(GLFWwindow* ) override;
    bool onLoad() override;
    void onRender() override;
    void onUpdate() override;
	void onDestroy() override;

private:
	UINT OpenDebug(void);
	void CreateDevice(UINT dxgiFactoryFlags, ComPtr<IDXGIFactory6>& factory,ComPtr<ID3D12Device>& device);  
	void CreateCommandQueue(ComPtr<ID3D12CommandQueue>& commandQueue);
	void CreateSwapChain(ID3D12CommandQueue* pQueue,int w,int h, ComPtr<IDXGISwapChain3>& swapChain3);
	
	void CreateRenderTargetViewHeap(ComPtr<ID3D12DescriptorHeap>& heap);
	void CreateRenderTargetView(ComPtr<ID3D12Resource> rtv[]);
	void CreateRootSignature(ComPtr<ID3D12RootSignature>& rootSignature);
	void CreateGPUPipelineState(ComPtr<ID3D12PipelineState>& pipelineState, ComPtr<ID3D12CommandAllocator>& commandAllocator, ComPtr<ID3D12GraphicsCommandList>& commandList);
	void InitializeVertexBuffer(ComPtr<ID3D12Resource>& vertexBuffer,D3D12_VERTEX_BUFFER_VIEW& vertexBufferView);
	
	void CreateCpuAndGpuSynchronization();
	void WaitForPreviousFrame(void);

	void PopulateCommandList(void);

	void ShadersCompileFromFile(LPCWSTR pFileName, LPCSTR pEntrypoint, LPCSTR pTarget, ComPtr<ID3DBlob>& shader);
	
	inline std::wstring GetShaderFilePath(LPCWSTR shaderName)
	{
		return m_CurrentPath + shaderName;
	}

	inline void GetCurrentPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
	{
		
		if (path == nullptr)
		{
			throw std::exception();
		}

		DWORD size = GetModuleFileNameW(nullptr, path, pathSize);
		if (size == 0 || size == pathSize)
		{
			// Method failed or path was truncated.
			throw std::exception();
		}

		WCHAR* lastSlash = wcsrchr(path, L'\\');
		if (lastSlash)
		{
			*(lastSlash + 1) = L'\0';
		}
	}

	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			
			throw HrException(hr);
		}
	}
private:
	static const UINT m_nFrameCount = 2;
	UINT m_frameIndex;
	HWND m_hWnd;
	ComPtr<IDXGIFactory6> m_factory;
	ComPtr<ID3D12Device> m_device;

	
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain3> m_swapChain;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;

	UINT m_rtvDescriptorSize;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12Resource> m_renderTargets[m_nFrameCount];

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;


	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;



	std::wstring m_CurrentPath;
};




#endif // HELLO_TRIANGLE_HXX
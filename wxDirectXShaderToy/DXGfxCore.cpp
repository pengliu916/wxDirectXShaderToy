
#include <math.h>
#include "DXGfxCore.h"

#pragma comment( lib, "d3d12.lib" )
#pragma comment(lib, "dxgi.lib")

// Helper Functions
void SetResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	D3D12_RESOURCE_BARRIER descBarrier;
	ZeroMemory(&descBarrier, sizeof(descBarrier));
	descBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	descBarrier.Transition.pResource = resource;
	descBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	descBarrier.Transition.StateBefore = before;
	descBarrier.Transition.StateAfter = after;
	commandList->ResourceBarrier(1, &descBarrier);
}

void DXGfxCore::WaitForCommandQueue(ID3D12CommandQueue* pCommandQueue) {
	static UINT64 frames = 0;
	queueFence->SetEventOnCompletion(frames, hFenceEvent);
	pCommandQueue->Signal(queueFence.Get(), frames);
	WaitForSingleObject(hFenceEvent, INFINITE);
	frames++;
}


DXGfxCore::DXGfxCore()
{

}

bool DXGfxCore::Init(HWND _hwnd)
{
	hwnd = _hwnd;
	return true;
}

bool DXGfxCore::CreateDevice()
{
	UINT flagsDXGI = 0;
	ID3D12Debug* debug = nullptr;
	HRESULT hr;
#if _DEBUG
	D3D12GetDebugInterface(IID_PPV_ARGS(&debug));
	if (debug) {
		debug->EnableDebugLayer();
		debug->Release();
	}
	flagsDXGI |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	hr = CreateDXGIFactory2(flagsDXGI, IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) {
		OutputDebugString(L"Failed CreateDXGIFactory2\n");
		return FALSE;
	}
	ComPtr<IDXGIAdapter> adapter;
	hr = dxgiFactory->EnumAdapters(0, adapter.GetAddressOf());
	if (FAILED(hr)) {
		return FALSE;
	}

	
	hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(dxDevice.GetAddressOf()));
	if (FAILED(hr)) {
		OutputDebugString(L"Failed D3D12CreateDevice\n");
		return FALSE;
	}
	
	hr = dxDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdAllocator.GetAddressOf()));
	if (FAILED(hr)) {
		OutputDebugString(L"Failed CreateCommandAllocator\n");
		return FALSE;
	}
	
	D3D12_COMMAND_QUEUE_DESC descCommandQueue;
	ZeroMemory(&descCommandQueue, sizeof(descCommandQueue));
	descCommandQueue.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	descCommandQueue.Priority = 0;
	descCommandQueue.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	hr = dxDevice->CreateCommandQueue(&descCommandQueue, IID_PPV_ARGS(cmdQueue.GetAddressOf()));
	if (FAILED(hr)) {
		OutputDebugString(L"Failed CreateCommandQueue\n");
		return FALSE;
	}
	
	hFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hr = dxDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(queueFence.GetAddressOf()));
	if (FAILED(hr)) {
		OutputDebugString(L"Failed CreateFence\n");
		return FALSE;
	}

	DXGI_SWAP_CHAIN_DESC descSwapChain;
	ZeroMemory(&descSwapChain, sizeof(descSwapChain));
	descSwapChain.BufferCount = 2;
	descSwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	descSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	descSwapChain.OutputWindow = hwnd;
	descSwapChain.SampleDesc.Count = 1;
	descSwapChain.Windowed = TRUE;
	descSwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	descSwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	hr = dxgiFactory->CreateSwapChain(cmdQueue.Get(), &descSwapChain, (IDXGISwapChain**)swapChain.GetAddressOf());
	if (FAILED(hr)) {
		OutputDebugString(L"Failed CreateSwapChain\n");
		return FALSE;
	}
	
	hr = dxDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator.Get(), nullptr, IID_PPV_ARGS(cmdList.GetAddressOf()));
	if (FAILED(hr)) {
		OutputDebugString(L"Failed CreateCommandList\n");
		return FALSE;
	}

	D3D12_DESCRIPTOR_HEAP_DESC descHeap;
	ZeroMemory(&descHeap, sizeof(descHeap));
	descHeap.NumDescriptors = 2;
	descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	hr = dxDevice->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(descriptorHeapRTV.GetAddressOf()));
	if (FAILED(hr)) {
		OutputDebugString(L"Failed CreateDescriptorHeap\n");
		return FALSE;
	}

	UINT strideHandleBytes = dxDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	for (UINT i = 0;i < descSwapChain.BufferCount;++i) {
		hr = swapChain->GetBuffer(i, IID_PPV_ARGS(renderTarget[i].GetAddressOf()));
		if (FAILED(hr)) {
			OutputDebugString(L"Failed swapChain->GetBuffer\n");
			return FALSE;
		}
		handleRTV[i] = descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart();
		handleRTV[i].ptr += i*strideHandleBytes;
		dxDevice->CreateRenderTargetView(renderTarget[i].Get(), nullptr, handleRTV[i]);
	}
	return TRUE;
}

bool DXGfxCore::Resize()
{
	return true;
}

void DXGfxCore::Update()
{

}

void DXGfxCore::Render()
{
	static int count = 0;
	float clearColor[4] = { 0,1.0f,1.0f,0 };
	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = 0; viewport.TopLeftY = 0;
	viewport.Width = 640;
	viewport.Height = 480;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	int targetIndex = swapChain->GetCurrentBackBufferIndex();

	SetResourceBarrier(
		cmdList.Get(),
		renderTarget[targetIndex].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	clearColor[0] = (float)(0.5f * sin(count*0.05f) + 0.5f);
	clearColor[1] = (float)(0.5f * sin(count*0.10f) + 0.5f);
	cmdList->RSSetViewports(1, &viewport);
	cmdList->ClearRenderTargetView(handleRTV[targetIndex], clearColor, 0, nullptr);

	SetResourceBarrier(
		cmdList.Get(),
		renderTarget[targetIndex].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);

	cmdList->Close();

	ID3D12CommandList* pCommandList = cmdList.Get();
	cmdQueue->ExecuteCommandLists(1, &pCommandList);
	swapChain->Present(1, 0);

	WaitForCommandQueue(cmdQueue.Get());
	cmdAllocator->Reset();
	cmdList->Reset(cmdAllocator.Get(), nullptr);
	count++;
}

void DXGfxCore::Release()
{

}

void DXGfxCore::Destory()
{

}
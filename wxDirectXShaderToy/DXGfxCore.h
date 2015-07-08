#pragma once
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_4.h>
//
// Handle to an Object
//
//
//#ifdef STRICT
//typedef void *HANDLE;
//#if 0 && (_MSC_VER > 1000)
//#define DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name
//#else
//#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
//#endif
//#else
//typedef PVOID HANDLE;
//#define DECLARE_HANDLE(name) typedef HANDLE name
//#endif
//typedef HANDLE *PHANDLE;

//typedef void* HANDLE;
using namespace Microsoft::WRL;
class DXGfxCore
{
public:
	DXGfxCore();
	bool Init(HWND);
	bool CreateDevice();
	bool Resize();
	void Update();
	void Render();
	void Release();
	void Destory();

private:
	void WaitForCommandQueue(ID3D12CommandQueue*);
	HWND								hwnd;
	HANDLE								hFenceEvent;
	
	ComPtr<ID3D12Device>				dxDevice;
	ComPtr<ID3D12CommandQueue>			cmdQueue;
	ComPtr<ID3D12CommandAllocator>		cmdAllocator;
	ComPtr<ID3D12GraphicsCommandList>	cmdList;
	ComPtr<IDXGISwapChain3>				swapChain;
	ComPtr<ID3D12Fence>					queueFence;
	ComPtr<IDXGIFactory3>				dxgiFactory;
	ComPtr<ID3D12DescriptorHeap>		descriptorHeapRTV;
	ComPtr<ID3D12Resource>				renderTarget[2];
	D3D12_CPU_DESCRIPTOR_HANDLE handleRTV[2];
};
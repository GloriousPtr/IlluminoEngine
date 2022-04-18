#include "ipch.h"
#include "ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>
#include <d3d12.h>

#include "Illumino/Core/Application.h"
#include "Window.h"

namespace IlluminoEngine
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach()
	{
		OPTICK_EVENT();

		Ref<Window> window = Application::GetApplication()->GetWindow();
		GraphicsContext* context = window->GetGraphicsContext().get();

		HWND hWnd = window->GetHwnd();
		ID3D12Device* device = reinterpret_cast<ID3D12Device*>(context->GetDevice());
		uint32_t framebufferCount = context->GetFrameCount();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

		io.ConfigWindowsMoveFromTitleBarOnly = true;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ID3D12DescriptorHeap* srvHeap = reinterpret_cast<ID3D12DescriptorHeap*>(context->GetSRVDescriptorHeap());
		// Setup Platform/Renderer backends
		D3D12_CPU_DESCRIPTOR_HANDLE cpuSrvHandle;
		cpuSrvHandle.ptr = srvHeap->GetCPUDescriptorHandleForHeapStart().ptr;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuSrvHandle;
		gpuSrvHandle.ptr = srvHeap->GetGPUDescriptorHandleForHeapStart().ptr;
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX12_Init(device, context->GetFrameCount(),
			DXGI_FORMAT_R8G8B8A8_UNORM, srvHeap,
			cpuSrvHandle,
			gpuSrvHandle);
	}

	void ImGuiLayer::OnDetach()
	{
		OPTICK_EVENT();

		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::Begin()
	{
		OPTICK_EVENT();

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		OPTICK_EVENT();

		Ref<Window> window = Application::GetApplication()->GetWindow();
		GraphicsContext* context = window->GetGraphicsContext().get();
		ID3D12GraphicsCommandList* commandList = reinterpret_cast<ID3D12GraphicsCommandList*>(context->GetCommandList());

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.DisplaySize = ImVec2((float)window->GetWidth(), (float)window->GetHeight());

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(NULL, commandList);
		}
	}
}
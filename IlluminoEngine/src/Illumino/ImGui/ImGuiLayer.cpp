#include "ipch.h"
#include "ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx12.h>
#include <d3d12.h>

#include "Platform/D3D12/Dx12GraphicsContext.h"
#include "Platform/D3D12/Dx12Resources.h"
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

	static DescriptorHandle s_ImGuiDescriptorHandle;
	void ImGuiLayer::OnAttach()
	{
		OPTICK_EVENT();

		Ref<Window> window = Application::GetApplication()->GetWindow();
		Dx12GraphicsContext* context = (Dx12GraphicsContext*)window->GetGraphicsContext().get();

		HWND hWnd = window->GetHwnd();
		ID3D12Device* device = reinterpret_cast<ID3D12Device*>(context->GetDevice());

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

		io.ConfigDockingTransparentPayload = true;
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		io.ConfigDragClickToInputText = true;

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

		DescriptorHeap& heap = context->GetSRVDescriptorHeap();
		s_ImGuiDescriptorHandle = heap.Allocate();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX12_Init(device, g_QueueSlotCount,
			DXGI_FORMAT_R8G8B8A8_UNORM, heap.GetHeap(),
			s_ImGuiDescriptorHandle.CPU,
			s_ImGuiDescriptorHandle.GPU);
	}

	void ImGuiLayer::OnDetach()
	{
		OPTICK_EVENT();

		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		Ref<Window> window = Application::GetApplication()->GetWindow();
		Dx12GraphicsContext* context = (Dx12GraphicsContext*)window->GetGraphicsContext().get();
		DescriptorHeap& heap = context->GetSRVDescriptorHeap();
		heap.Free(s_ImGuiDescriptorHandle);
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
		Dx12GraphicsContext* context = (Dx12GraphicsContext*) window->GetGraphicsContext().get();
		ID3D12GraphicsCommandList* commandList = context->GetCommandList();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.DisplaySize = ImVec2((float)window->GetWidth(), (float)window->GetHeight());

		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(NULL, commandList);
		}
	}
}

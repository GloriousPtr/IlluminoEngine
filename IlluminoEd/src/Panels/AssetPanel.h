#pragma once

#include <IlluminoEngine.h>

#include <imgui/imgui.h>

#include "../Utils/IconsMaterialDesignIcons.h"

namespace IlluminoEngine
{
	class AssetPanel
	{
	public:
		AssetPanel();

		virtual ~AssetPanel() = default;

		virtual void OnImGuiRender();

	private:
		std::pair<bool, uint32_t> DirectoryTreeViewRecursive(const std::filesystem::path& path, uint32_t* count, int* selection_mask);
		void RenderHeader();
		void RenderSideView();
		void RenderBody();
		void UpdateDirectoryEntries(std::filesystem::path directory);

	private:
		struct File
		{
			eastl::string Name;
			std::filesystem::directory_entry DirectoryEntry;
			Ref<Texture2D> IconTexture;
		};

		std::filesystem::path m_CurrentDirectory;
		eastl::vector<File> m_DirectoryEntries;
		uint32_t m_CurrentlyVisibleItemsTreeView = 0;
		float m_TreeViewColumnWidth = 256.0f;
		float m_ThumbnailSize = 128.0f;
		ImGuiTextFilter m_Filter;

		Ref<Texture2D> m_DirectoryIcon;
	};
}

#pragma once

#include "Illumino/Core/Core.h"

namespace IlluminoEngine
{
	enum class ShaderDataClassification
	{
		Vertex = 0, Instance
	};

	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:		return 4;
			case ShaderDataType::Float2:	return 4 * 2;
			case ShaderDataType::Float3:	return 4 * 3;
			case ShaderDataType::Float4:	return 4 * 4;
			case ShaderDataType::Mat3:		return 4 * 3 * 3;
			case ShaderDataType::Mat4:		return 4 * 4 * 4;
			case ShaderDataType::Int:		return 4;
			case ShaderDataType::Int2:		return 4 * 2;
			case ShaderDataType::Int3:		return 4 * 3;
			case ShaderDataType::Int4:		return 4 * 4;
			case ShaderDataType::Bool:		return 1;
		}

		ILLUMINO_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct BufferElement
	{
		String Name;
		ShaderDataType Type;
		ShaderDataClassification Classification;
		uint32_t Size;
		size_t Offset;

		BufferElement() = default;

		BufferElement(String&& name, ShaderDataType type, ShaderDataClassification classification = ShaderDataClassification::Vertex)
			: Name((String&&)name), Type(type), Classification(classification), Size(ShaderDataTypeSize(type)), Offset(0)
		{
		}

		uint32_t GetComponentCount()
		{
			switch (Type)
			{
				case ShaderDataType::Float:		return 1;
				case ShaderDataType::Float2:	return 2;
				case ShaderDataType::Float3:	return 3;
				case ShaderDataType::Float4:	return 4;
				case ShaderDataType::Mat3:		return 3; // 3 * float 3;
				case ShaderDataType::Mat4:		return 4; // 4 * float 4;
				case ShaderDataType::Int:		return 1;
				case ShaderDataType::Int2:		return 2;
				case ShaderDataType::Int3:		return 3;
				case ShaderDataType::Int4:		return 4;
				case ShaderDataType::Bool:		return 1;
			}

			ILLUMINO_ASSERT(false, "Unknown ShaderDataType!");
			return 0;
		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement>& elements)
			: m_Elements(elements)
		{
			CalculateOffsetAndStride();
		}

		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		inline size_t GetCount() const { return m_Elements.size(); }
		inline size_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetAndStride()
		{
			size_t offset = 0;
			m_Stride = 0;
			for(auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride;
	};

	class MeshBuffer
	{
	public:
		virtual ~MeshBuffer() = default;

		virtual void* GetVertexBufferView() = 0;
		virtual void* GetIndexBufferView() = 0;
		virtual void Bind() = 0;

		virtual uint32_t GetVertexCount() = 0;
		virtual uint32_t GetIndexCount() = 0;

		static Ref<MeshBuffer> Create(float* vertexData, uint32_t* indexData, size_t verticesSize, size_t indicesSize, size_t strideSize);
	};
}

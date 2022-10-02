#pragma once

/*
 * The stride of a buffers's layout is basically the size of all its attributes combined, since the vertexAtribs are not being
 * sorted into each type first so that works out nicely, eventhough that would have the same stride though we would have an
 * offset parameter for different elements
 */

#include "Core/Base.h"

#include <string>
#include <initializer_list>
#include <vector>

namespace Aurora {

	enum class ShaderDataType
	{
		None = 0,
		Float,
		Float2,
		Float3,
		Float4,
		Mat3,
		Mat4,
		Bool,
		UInt,
		Int,
		Int2,
		Int3,
		Int4
	};

	//////////////////////////
	// BUFFER ELEMENT!!
	//////////////////////////

	struct BufferElement
	{
		std::string name;
		ShaderDataType type;
		uint32_t size;
		size_t offset;
		bool normalized;

		BufferElement() = default;
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false);

		[[nodiscard]] uint32_t GetComponentCount() const;

	};

	//////////////////////////
	// BUFFER LAYOUT!!
	//////////////////////////

	class BufferLayout
	{
	public:
		BufferLayout()
			: m_Elements(), m_Stride(0) {}

		BufferLayout(const std::initializer_list<BufferElement>& initList)
			: m_Elements(initList)
		{
			CalcStrideAndOffset();
		}

		[[nodiscard]] inline uint32_t GetStride() const { return m_Stride; }
		[[nodiscard]] inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		inline std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		inline std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		inline std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		inline std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalcStrideAndOffset();

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride;

	};

	//////////////////////////
	// VERTEX BUFFER!!
	//////////////////////////

	enum class BufferUsage
	{
		None = 0,
		Static,
		Dynamic
	};

	class VertexBuffer : public RefCountedObject
	{
	public:
		VertexBuffer() = default;
		VertexBuffer(uint32_t size, BufferUsage drawHint = BufferUsage::Static);
		VertexBuffer(void* vertices, uint32_t size, BufferUsage drawHint = BufferUsage::Static);
		virtual ~VertexBuffer();

		[[nodiscard]] static Ref<VertexBuffer> Create(uint32_t size, BufferUsage drawHint = BufferUsage::Static);
		[[nodiscard]] static Ref<VertexBuffer> Create(void* vertices, uint32_t size, BufferUsage drawHint = BufferUsage::Static);

		void Bind() const;
		void UnBind() const;

		void SetData(const void* data, uint32_t size);

		[[nodiscard]] inline const BufferLayout& GetBufferLayout() const { return m_Layout; }
		void SetLayout(const BufferLayout& layout) { m_Layout = layout; }


	private:
		uint32_t m_BufferID;
		BufferLayout m_Layout;

	};

}
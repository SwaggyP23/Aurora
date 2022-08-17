#pragma once

#include "Core/Base.h"

#include <string>
#include <initializer_list>
#include <vector>

namespace Aurora {

	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:   return 4;
			case ShaderDataType::Float2:  return 4 * 2;
			case ShaderDataType::Float3:  return 4 * 3;
			case ShaderDataType::Float4:  return 4 * 4;
			case ShaderDataType::Mat3:	  return 4 * 3 * 3;
			case ShaderDataType::Mat4:    return 4 * 4 * 4;
			case ShaderDataType::Int:	  return 4;
			case ShaderDataType::Int2:	  return 4 * 2;
			case ShaderDataType::Int3:	  return 4 * 3;
			case ShaderDataType::Int4:	  return 4 * 4;
			case ShaderDataType::Bool:    return 1;
		}

		AR_CORE_ASSERT(false, "Unknown Shader Data Type!");
		return 0;
	}

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

		uint32_t GetComponentCount() const;

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

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalcStrideAndOffset();

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride;

	};

	//////////////////////////
	// VERTEX BUFFER!!
	//////////////////////////

	enum class VertexBufferDrawHint
	{
		None = 0,
		Static,
		Dynamic
	};

	class VertexBuffer : public RefCountedObject
	{
	public:
		VertexBuffer() = default;
		VertexBuffer(uint32_t size, VertexBufferDrawHint drawHint = VertexBufferDrawHint::Static);
		VertexBuffer(float* vertices, uint32_t size, VertexBufferDrawHint drawHint = VertexBufferDrawHint::Static);
		~VertexBuffer();

		static Ref<VertexBuffer> Create(uint32_t size, VertexBufferDrawHint drawHint = VertexBufferDrawHint::Static);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size, VertexBufferDrawHint drawHint = VertexBufferDrawHint::Static);

		void Bind() const;
		void UnBind() const;

		void SetData(const void* data, uint32_t size);

		inline const BufferLayout& GetBufferLayout() const { return m_Layout; }
		void SetLayout(const BufferLayout& layout) { m_Layout = layout; }


	private:
		uint32_t m_BufferID;
		BufferLayout m_Layout;
		VertexBufferDrawHint m_DrawHint;

	};

	//////////////////////////
	// INDEX BUFFER!!
	//////////////////////////

	// TODO: Only supports 32-bit index buffers, should add support for 16-bit index buffers
	class IndexBuffer : public RefCountedObject
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(uint32_t* indices, uint32_t count);
		~IndexBuffer();

		static Ref<IndexBuffer> Create();
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);

		void Bind() const;
		void UnBind() const;

		inline uint32_t GetCount() const { return m_Count; }

	private:
		uint32_t m_BufferId = 0;
		uint32_t m_Count = 0;

	};

}
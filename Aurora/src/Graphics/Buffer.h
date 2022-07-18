#pragma once

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "Aurora.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

		AR_CORE_ASSERT(false, "Unkown Shader Data Type!");
		return 0;
	}

	static uint32_t ShaderDataTypeToOpenGLType(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:   return GL_FLOAT;
		case ShaderDataType::Float2:  return GL_FLOAT;
		case ShaderDataType::Float3:  return GL_FLOAT;
		case ShaderDataType::Float4:  return GL_FLOAT;
		case ShaderDataType::Mat3:	  return GL_FLOAT;
		case ShaderDataType::Mat4:    return GL_FLOAT;
		case ShaderDataType::Int:	  return GL_INT;
		case ShaderDataType::Int2:	  return GL_INT;
		case ShaderDataType::Int3:	  return GL_INT;
		case ShaderDataType::Int4:	  return GL_INT;
		case ShaderDataType::Bool:    return GL_BOOL;
		}

		AR_CORE_ASSERT(false, "Unkown Shader Data Type!");
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

		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized)
		{
		}

		uint32_t getComponentCount() const;
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
			calcStrideAndOffset();
		}

		inline uint32_t getStride() const { return m_Stride; }
		inline const std::vector<BufferElement>& getElements() const { return m_Elements; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

	private:
		void calcStrideAndOffset();

	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride;
	};

	//////////////////////////
	// VERTEX BUFFER!!
	//////////////////////////

	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		VertexBuffer(uint32_t size);
		VertexBuffer(float* vertices, uint32_t size);
		~VertexBuffer();

		static Ref<VertexBuffer> Create(uint32_t size);
		static Ref<VertexBuffer> Create(float* vertices, uint32_t size);

		void bind() const;
		void unBind() const;

		void SetData(const void* data, uint32_t size);

		inline const BufferLayout& getBufferLayout() const { return m_Layout; }
		void setLayout(const BufferLayout& layout) { m_Layout = layout; }


	private:
		uint32_t m_BufferID;
		BufferLayout m_Layout;
	};

	//////////////////////////
	// INDEX BUFFER!!
	//////////////////////////

	// Only supports 32-bit index buffers
	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		IndexBuffer(uint32_t* indices, uint32_t count);
		~IndexBuffer();

		static Ref<IndexBuffer> Create();
		static Ref<IndexBuffer> Create(uint32_t* indices, uint32_t count);

		void bind() const;
		void unBind() const;

		inline uint32_t getCount() const { return m_Count; }

	private:
		uint32_t m_BufferId;
		uint32_t m_Count;
	};

}

#endif // !_BUFFER_H_
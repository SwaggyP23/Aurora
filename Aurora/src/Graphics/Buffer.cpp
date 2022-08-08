#include "Aurorapch.h"
#include "Buffer.h"

#include <glad/glad.h>

namespace Aurora {

	namespace Utils {

		static GLenum GLDrawHintTypeFromEnum(VertexBufferDrawHint hint)
		{
			switch (hint)
			{
			    case Aurora::VertexBufferDrawHint::None:        return GL_NONE;
			    case Aurora::VertexBufferDrawHint::Static:      return GL_STATIC_DRAW;
			    case Aurora::VertexBufferDrawHint::Dynamic:     return GL_DYNAMIC_DRAW;
			}

			AR_CORE_ASSERT(false, "[Buffer]: Unkown draw hint type!");
			return 0;
		}

	}

	//////////////////////////
	// BUFFER ELEMENT!!
	//////////////////////////

	BufferElement::BufferElement(ShaderDataType type, const std::string& name, bool normalized)
		: name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized)
	{
	}

	uint32_t BufferElement::GetComponentCount() const
	{
		switch (type)
		{
		    case ShaderDataType::Float:   return 1;
		    case ShaderDataType::Float2:  return 2;
		    case ShaderDataType::Float3:  return 3;
		    case ShaderDataType::Float4:  return 4;
		    case ShaderDataType::Mat3:    return 3 * 3;
		    case ShaderDataType::Mat4:    return 4 * 4;
		    case ShaderDataType::Int:     return 1;
		    case ShaderDataType::Int2:    return 2;
		    case ShaderDataType::Int3:    return 3;
		    case ShaderDataType::Int4:    return 4;
		    case ShaderDataType::Bool:    return 1;
		}

		AR_CORE_ASSERT(false, "[Buffer]: Unkown Shader Data Type!");
		return 0;
	}

	//////////////////////////
	// BUFFER LAYOUT!!
	//////////////////////////

	void BufferLayout::CalcStrideAndOffset()
	{
		AR_PROFILE_FUNCTION();

		GLuint offset = 0;
		m_Stride = 0;
		for (auto& element : m_Elements)
		{
			element.offset = offset;
			offset += element.size;
			m_Stride += element.size;
		}
	}

	//////////////////////////
	// VERTEX BUFFER!!
	//////////////////////////

	Ref<VertexBuffer> VertexBuffer::Create(uint32_t size, VertexBufferDrawHint drawHint)
	{
		return CreateRef<VertexBuffer>(size, drawHint);
	}

	Ref<VertexBuffer> VertexBuffer::Create(float* vertices, uint32_t size, VertexBufferDrawHint drawHint)
	{
		return CreateRef<VertexBuffer>(vertices, size, drawHint);
	}

	VertexBuffer::VertexBuffer(uint32_t size, VertexBufferDrawHint drawHint)
	{
		AR_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_BufferID);
		glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, Utils::GLDrawHintTypeFromEnum(drawHint));
	}

	VertexBuffer::VertexBuffer(float* vertices, uint32_t size, VertexBufferDrawHint drawHint)
	{
		AR_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_BufferID);
		glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
		glBufferData(GL_ARRAY_BUFFER, size, (const void*)vertices, Utils::GLDrawHintTypeFromEnum(drawHint));
	}

	VertexBuffer::~VertexBuffer()
	{
		AR_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_BufferID);
	}

	void VertexBuffer::Bind() const
	{
		AR_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
	}

	void VertexBuffer::UnBind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void VertexBuffer::SetData(const void* data, uint32_t size)
	{
		AR_PROFILE_FUNCTION();

		glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}

	//////////////////////////
	// INDEX BUFFER!!
	//////////////////////////

	Ref<IndexBuffer> IndexBuffer::Create()
	{
		return CreateRef<IndexBuffer>();
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32_t* indices, uint32_t count)
	{
		return CreateRef<IndexBuffer>(indices, count);
	}

	IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
		: m_Count(count)
	{
		AR_PROFILE_FUNCTION();

		glCreateBuffers(1, &m_BufferId);
		// Since we specified here that it is a GL_ELEMENT_ARRAY_BUFFER, a VAO must be bound when this is created 
		// otherwise it will not work
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
	}

	IndexBuffer::~IndexBuffer()
	{
		AR_PROFILE_FUNCTION();

		glDeleteBuffers(1, &m_BufferId);
	}

	void IndexBuffer::Bind() const
	{
		AR_PROFILE_FUNCTION();

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferId);
	}

	void IndexBuffer::UnBind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}
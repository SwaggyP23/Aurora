#include "Aurorapch.h"
#include "VertexArray.h"

#include <glad/glad.h>

namespace Aurora {

	static uint32_t ShaderDataTypeToOpenGLType(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:	        return GL_FLOAT;
			case ShaderDataType::Float2:        return GL_FLOAT;
			case ShaderDataType::Float3:        return GL_FLOAT;
			case ShaderDataType::Float4:        return GL_FLOAT;
			case ShaderDataType::Mat3:	        return GL_FLOAT;
			case ShaderDataType::Mat4:	        return GL_FLOAT;
			case ShaderDataType::Int:	        return GL_INT;
			case ShaderDataType::Int2:	        return GL_INT;
			case ShaderDataType::Int3:	        return GL_INT;
			case ShaderDataType::Int4:	        return GL_INT;
			case ShaderDataType::Bool:	        return GL_BOOL;
		}

		AR_CORE_ASSERT(false, "Unkown Shader Data Type!");
		return 0;
	}

	Ref<VertexArray> VertexArray::Create()
	{
		return CreateRef<VertexArray>();
	}

	VertexArray::VertexArray()
		: m_IndexBuffer()
	{
		AR_PROFILE_FUNCTION();

		glCreateVertexArrays(1, &m_ArrayId);
	}

	VertexArray::~VertexArray()
	{
		AR_PROFILE_FUNCTION();

		glDeleteVertexArrays(1, &m_ArrayId);
	}

	void VertexArray::bind() const
	{
		AR_PROFILE_FUNCTION();

		glBindVertexArray(m_ArrayId);
	}

	void VertexArray::unBind() const
	{
		glBindVertexArray(0);
	}

	void VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(vertexBuffer->GetBufferLayout().GetElements().size(), "Vertex Buffer had no layout!");

		glBindVertexArray(m_ArrayId);
		vertexBuffer->bind();

		int index = 0;
		const auto& layout = vertexBuffer->GetBufferLayout();
		for (const auto& element : layout)
		{
			glVertexAttribPointer(index,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLType(element.type),
				element.normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.offset);

			glEnableVertexAttribArray(index++);
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		AR_PROFILE_FUNCTION();

		glBindVertexArray(m_ArrayId);
		indexBuffer->bind();

		m_IndexBuffer = indexBuffer;
	}

}
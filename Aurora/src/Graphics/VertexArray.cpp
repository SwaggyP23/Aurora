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

	void VertexArray::Bind() const
	{
		AR_PROFILE_FUNCTION();

		glBindVertexArray(m_ArrayId);
	}

	void VertexArray::UnBind() const
	{
		glBindVertexArray(0);
	}

	void VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer)
	{
		AR_PROFILE_FUNCTION();

		AR_CORE_ASSERT(vertexBuffer->GetBufferLayout().GetElements().size(), "Vertex Buffer had no layout!");

		glBindVertexArray(m_ArrayId);
		vertexBuffer->Bind();

		int index = 0;
		const auto& layout = vertexBuffer->GetBufferLayout();
		for (const auto& element : layout)
		{
			switch (element.type)
			{
			    case ShaderDataType::Float:
			    case ShaderDataType::Float2:
			    case ShaderDataType::Float3:
			    case ShaderDataType::Float4:
			    {
			    	glVertexAttribPointer(index,
			    		element.GetComponentCount(),
			    		ShaderDataTypeToOpenGLType(element.type),
			    		element.normalized ? GL_TRUE : GL_FALSE,
			    		layout.GetStride(),
			    		(const void*)element.offset);
			    
			    	glEnableVertexAttribArray(index++);
			    	break;
			    }
				case ShaderDataType::Int:
				case ShaderDataType::Int2:
				case ShaderDataType::Int3:
				case ShaderDataType::Int4:
				case ShaderDataType::Bool:
				{
					glVertexAttribIPointer(index,
						element.GetComponentCount(),
						ShaderDataTypeToOpenGLType(element.type),
						layout.GetStride(),
						(const void*)element.offset);

					glEnableVertexAttribArray(index++);
					break;
				}
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4: // Dunno what the F is this i just copied
				{
					uint8_t count = element.GetComponentCount();
					for (uint8_t i = 0; i < count; i++)
					{
						glVertexAttribPointer(index,
							count,
							ShaderDataTypeToOpenGLType(element.type),
							element.normalized ? GL_TRUE : GL_FALSE,
							layout.GetStride(),
							(const void*)(element.offset + sizeof(float) * count * i));
						glVertexAttribDivisor(index, 1);

						glEnableVertexAttribArray(index++);
					}

					break;
				}
			}

			
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		AR_PROFILE_FUNCTION();

		glBindVertexArray(m_ArrayId);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}
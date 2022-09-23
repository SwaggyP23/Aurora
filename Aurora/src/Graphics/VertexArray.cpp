#include "Aurorapch.h"
#include "VertexArray.h"

#include <glad/glad.h>

namespace Aurora {

	namespace Utils {

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
				case ShaderDataType::UInt:			return GL_UNSIGNED_INT;
			    case ShaderDataType::Int:	        return GL_INT;
			    case ShaderDataType::Int2:	        return GL_INT;
			    case ShaderDataType::Int3:	        return GL_INT;
			    case ShaderDataType::Int4:	        return GL_INT;
			    case ShaderDataType::Bool:	        return GL_BOOL;
			}

			AR_CORE_ASSERT(false, "Unknown Shader Data Type!");
			return 0;
		}

	}

	Ref<VertexArray> VertexArray::Create()
	{
		return CreateRef<VertexArray>();
	}

	VertexArray::VertexArray()
		: m_IndexBuffer()
	{
		glCreateVertexArrays(1, &m_ArrayId);
	}

	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(1, &m_ArrayId);
	}

	void VertexArray::Bind() const
	{
		glBindVertexArray(m_ArrayId);
	}

	void VertexArray::UnBind() const
	{
		glBindVertexArray(0);
	}

	// TODO: Rework to support Instanced Rendering...
	void VertexArray::AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer, uint32_t index)
	{
		AR_CORE_ASSERT(vertexBuffer->GetBufferLayout().GetElements().size(), "Vertex Buffer has no layout!");

		glBindVertexArray(m_ArrayId);
		vertexBuffer->Bind();

		uint32_t atribIndex = 0;
		const BufferLayout& layout = vertexBuffer->GetBufferLayout();
		for (const BufferElement& element : layout)
		{
			switch (element.type)
			{
			    case ShaderDataType::Float:
			    case ShaderDataType::Float2:
			    case ShaderDataType::Float3:
			    case ShaderDataType::Float4:
			    {
			    	glVertexAttribPointer(atribIndex,
			    		element.GetComponentCount(),
			    		Utils::ShaderDataTypeToOpenGLType(element.type),
			    		element.normalized ? GL_TRUE : GL_FALSE,
			    		layout.GetStride(),
			    		(const void*)element.offset);
			    
			    	glEnableVertexAttribArray(atribIndex++);
			    	break;
			    }
				case ShaderDataType::UInt:
				case ShaderDataType::Int:
				case ShaderDataType::Int2:
				case ShaderDataType::Int3:
				case ShaderDataType::Int4:
				case ShaderDataType::Bool:
				{
					glVertexAttribIPointer(atribIndex,
						element.GetComponentCount(),
						Utils::ShaderDataTypeToOpenGLType(element.type),
						layout.GetStride(),
						(const void*)element.offset);

					glEnableVertexAttribArray(atribIndex++);
					break;
				}
				case ShaderDataType::Mat3:
				case ShaderDataType::Mat4:
				{
					uint32_t count = element.GetComponentCount();
					for (uint32_t i = 0; i < count; i++)
					{
						glVertexAttribPointer(atribIndex,
							count,
							Utils::ShaderDataTypeToOpenGLType(element.type),
							element.normalized ? GL_TRUE : GL_FALSE,
							layout.GetStride(),
							(const void*)(element.offset + sizeof(float) * count * i));
						glVertexAttribDivisor(atribIndex, 1);

						glEnableVertexAttribArray(atribIndex++);
					}

					break;
				}
			}
		}

		m_VertexBuffers[index] = vertexBuffer;
	}

	void VertexArray::SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer)
	{
		// With the OpenGL new API and DSA I could use this instead of binding both buffers and that means they are bound...
		glVertexArrayElementBuffer(m_ArrayId, indexBuffer->GetBufferID());

		m_IndexBuffer = indexBuffer;
	}

}
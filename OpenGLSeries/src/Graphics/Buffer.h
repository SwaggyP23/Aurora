#pragma once

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "OGLpch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

enum class ShaderDataType
{
	None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4
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
	}

	CORE_LOG_ERROR("Unkown Shader Data Type!! {0}", __FUNCTION__);
	return 0;
}

static GLenum ShaderDataTypeToOpenGLType(ShaderDataType type)
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
	}

	CORE_LOG_ERROR("Unkown Shader Data Type!! {0}", __FUNCTION__);
	return 0;
}

//////////////////////////
// BUFFER ELEMENT!!
//////////////////////////

struct BufferElement
{
	std::string name;
	ShaderDataType type;
	GLuint size;
	size_t offset;
	bool normalized;

	BufferElement() = default;

	BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
		: name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalized(normalized)
	{
	}

	GLuint getComponentCount() const;
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

	inline GLuint getStride() const { return m_Stride; }
	inline const std::vector<BufferElement>& getElements() const { return m_Elements; }

	std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
	std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
	std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
	std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

private:
	void calcStrideAndOffset();

private:
	std::vector<BufferElement> m_Elements;
	GLuint m_Stride;
};

//////////////////////////
// VERTEX BUFFER!!
//////////////////////////

class VertexBuffer
{
public:
	VertexBuffer() = default;
	VertexBuffer(GLfloat* vertices, size_t size);
	~VertexBuffer();

	void bind() const;
	void unBind() const;

	void setLayout(const BufferLayout& layout);

	inline BufferLayout getBufferLayout() const { return m_Layout; }

private:
	GLuint m_BufferID;
	BufferLayout m_Layout;
};

//////////////////////////
// INDEX BUFFER!!
//////////////////////////

class IndexBuffer
{
public:
	IndexBuffer() = default;
	IndexBuffer(GLuint* indices, size_t count);
	~IndexBuffer();

	void bind() const;
	void unBind() const;

	inline size_t getCount() const { return m_Count; }

private:
	GLuint m_BufferId;
	size_t m_Count;
};

#endif // !_BUFFER_H_
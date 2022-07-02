#pragma once

#ifndef _VERTEX_ARRAY_H_
#define _VERTEX_ARRAY_H_

#include "OGLpch.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Buffer.h"

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void addVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
	void setIndexBuffer(const Ref<IndexBuffer>& indexBuffer);

	void bind() const;
	void unBind() const;

	inline const std::vector<Ref<VertexBuffer>>& getVertexBuffers() const { return m_VertexBuffers; }
	inline const Ref<IndexBuffer>& getIndexBuffer() const { return m_IndexBuffer; }

private:
	GLuint m_ArrayId;
	std::vector<Ref<VertexBuffer>> m_VertexBuffers;
	Ref<IndexBuffer> m_IndexBuffer;

};

#endif // !_VERTEX_ARRAY_H_
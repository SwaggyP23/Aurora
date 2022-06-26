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

	void addVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexBuffer);
	void setIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

	void bind() const;
	void unBind() const;

	inline const std::vector<std::shared_ptr<VertexBuffer>>& getVertexBuffers() const { return m_VertexBuffers; }
	inline const std::shared_ptr<IndexBuffer>& getIndexBuffer() const { return m_IndexBuffer; }

private:
	GLuint m_ArrayId;
	std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
	std::shared_ptr<IndexBuffer> m_IndexBuffer;

};

#endif // !_VERTEX_ARRAY_H_
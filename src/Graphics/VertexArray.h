#pragma once

#ifndef _VERTEX_ARRAY_H_
#define _VERTEX_ARRAY_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Buffer.h"

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void addVertexBuffer(const VertexBuffer& vertexbuffer);
	void setIndexBuffer(const IndexBuffer& indexbuffer);

	void bind() const;
	void unBind() const;

	inline const std::vector<VertexBuffer>& getVertexBuffers() const { return m_VertexBuffers; }
	inline const IndexBuffer& getIndexBuffer() const { return m_IndexBuffer; }

private:
	GLuint m_ArrayId;
	std::vector<VertexBuffer> m_VertexBuffers;
	IndexBuffer m_IndexBuffer;

};

#endif // !_VERTEX_ARRAY_H_
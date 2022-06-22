#pragma once

#ifndef _BUFFER_H_
#define _BUFFER_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class VertexBuffer
{
private:
	GLuint m_BufferID;

public:

	void bind() const;
	void unBind() const;
};

class IndexBuffer
{
private:
	GLuint m_BufferId;
	GLuint m_Count;

public:


	void bind() const;
	void unBind() const;

	inline GLuint getCount() const { return m_Count; }
};

#endif // !_BUFFER_H_
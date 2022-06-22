#pragma once

#ifndef _VERTEX_ARRAY_H_
#define _VERTEX_ARRAY_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class VertexArray
{
private:
	GLuint m_ArrayId;

public:


	void bind() const;
	void unBind() const;
};

#endif // !_VERTEX_ARRAY_H_
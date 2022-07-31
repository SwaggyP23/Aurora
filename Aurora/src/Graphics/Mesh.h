#pragma once

// This is to be continued when reworking the shader to system to use Spir-V and Uniform Buffers that way meshes and materials are easier

#include "Core/Base.h"
#include "Buffer.h"
#include "VertexArray.h"

#include <glm/glm.hpp>

namespace Aurora {

	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	};

	class Mesh
	{
	public:
		Mesh(const Ref<VertexArray>& vao, const Ref<IndexBuffer>& ibo/*TODO: material*/);
		Mesh(const Mesh& other);
		~Mesh();

		void Draw();

	private:
		Ref<VertexArray> m_VertexArray; // This will need to include a vertex buffer and an index buffer
		Ref<IndexBuffer> m_IndexBuffer;
		// TODO: m_Material

	};

}
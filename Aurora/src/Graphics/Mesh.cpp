#include "Aurorapch.h"
#include "Mesh.h"

#include "Renderer/RenderCommand.h"

namespace Aurora {

	Mesh::Mesh(const Ref<VertexArray>& vao, const Ref<IndexBuffer>& ibo/*TODO: material*/)
		: m_VertexArray(vao), m_IndexBuffer(ibo)
	{
	}

	Mesh::Mesh(const Mesh& other)
		: m_VertexArray(other.m_VertexArray), m_IndexBuffer(other.m_IndexBuffer)
	{
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Draw()
	{
		m_VertexArray->Bind();
		m_IndexBuffer->Bind();

		RenderCommand::DrawIndexed(m_VertexArray, m_IndexBuffer->GetCount());

		m_IndexBuffer->UnBind();
		m_VertexArray->UnBind();
	}

}
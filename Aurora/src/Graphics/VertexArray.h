#pragma once

/*
 * TODO: Rework to support Instanced Rendering...
 */

#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace Aurora {

	class VertexArray : public RefCountedObject
	{
	public:
		VertexArray();
		virtual ~VertexArray();

		[[nodiscard]] static Ref<VertexArray> Create();

		void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer, uint32_t index = 0);
		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);

		void Bind() const;
		void UnBind() const;

		[[nodiscard]] inline const std::array<Ref<VertexBuffer>, 2>& GetVertexBuffers() const { return m_VertexBuffers; }
		[[nodiscard]] inline const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		uint32_t m_ArrayId = 0;
		// Only allow for 2 VBOs at most for each vertex array object eventhough you will only use 1
		std::array<Ref<VertexBuffer>, 2> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer = nullptr;

	};

}
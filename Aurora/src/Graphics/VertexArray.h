#pragma once

#include "Buffer.h"

namespace Aurora {

	class VertexArray : public RefCountedObject
	{
	public:
		VertexArray();
		~VertexArray();

		static Ref<VertexArray> Create();

		void AddVertexBuffer(const Ref<VertexBuffer>& vertexBuffer);
		void SetIndexBuffer(const Ref<IndexBuffer>& indexBuffer);

		void Bind() const;
		void UnBind() const;

		inline const std::vector<Ref<VertexBuffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		inline const Ref<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

	private:
		uint32_t m_ArrayId;
		std::vector<Ref<VertexBuffer>> m_VertexBuffers;
		Ref<IndexBuffer> m_IndexBuffer;

	};

}
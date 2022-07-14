#pragma once

#include "Core/Base.h"

namespace Aurora {

	struct FramebufferSpecification
	{
		uint32_t Width, Height;
		uint32_t Samples = 1;

		bool SwapChainTarget = false; // This is the equivalent of glBindFramebuffer(0);
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& spec);
		~Framebuffer();

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);

		void Invalidate();

		void bind() const;
		void unBind() const;

		const FramebufferSpecification& GetSpecification() const { return m_Specification; }
		uint32_t GetColorAttachmentID() const { return m_ColorAttachment; }

	private:
		uint32_t m_BufferID;
		uint32_t m_ColorAttachment;
		uint32_t m_DepthAttachment;
		FramebufferSpecification m_Specification;

	};

}